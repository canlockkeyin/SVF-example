#include "SVF-FE/LLVMUtil.h"
#include "Graphs/SVFG.h"
#include "WPA/Andersen.h"

SVFModule* svfModule;
PTACallGraph* callgraph;

std::string printVal(const Value* v) {
    std::string val_str;
    raw_string_ostream rso(val_str);
    v->print(rso);
    return rso.str();
}

void getCallPaths(const Function* f, std::vector<std::vector<const Instruction*>>& paths, std::vector<const Instruction*>& curPath) {
    if (const SVFFunction* sf = svfModule->getSVFFunction(f)) {
        PTACallGraphNode* cgn = callgraph->getCallGraphNode(sf);
        /* check if further path exists */
        if (cgn->hasIncomingEdge()) {
            /* incoming edges of this node */
            for (auto edgit = cgn->InEdgeBegin(); edgit != cgn->InEdgeEnd(); ++edgit) {
                PTACallGraphEdge* edg = *edgit;
                PTACallGraphEdge::CallInstSet cis = edg->getDirectCalls();
                for (const CallBlockNode* cbn : cis) {
                    const Function* cf = cbn->getCallSite()->getFunction();
                    //const Instruction* ci = cbn->getCallSite()->getInstruction();
                    const Instruction* ci = cbn->getCallSite();
                    curPath.push_back(ci);
                    getCallPaths(cf, paths, curPath);
                    /* when the recursion above finishes, we have reached a root node
                     * therefore we go back (down) one node and check its other parents
                     */
                    curPath.pop_back();
                }
            }
        }
        /* end of this callpath reached */
        else {
            paths.push_back(curPath);
        }
    }
}

void collectUsesOnVFG(const SVFG& vfg, const Value& val){
	PAG* pag = PAG::getPAG();

    PAGNode* pNode = pag->getPAGNode(pag->getValueNode(&val));
    const VFGNode* vNode = vfg.getDefSVFGNode(pNode);
    FIFOWorkList<const VFGNode*> worklist;
    std::set<const VFGNode*> visited;
    worklist.push(vNode);

    /// Traverse along VFG
    while(!worklist.empty()){
        const VFGNode* vNode = worklist.pop();
        visited.insert(vNode);
        for(VFGNode::const_iterator it = vNode->OutEdgeBegin(); it != vNode->OutEdgeEnd(); ++it) {
            if(visited.find((*it)->getDstNode())==visited.end()){
                worklist.push((*it)->getDstNode());
            }
        }
    }
    /// Collect all LLVM Values
    for(std::set<const VFGNode*>::const_iterator it = visited.begin(), eit = visited.end(); it!=eit; ++it){
    	const VFGNode* node = *it;
    	/// can only query VFGNode involving top-level pointers (starting with % or @ in LLVM IR)
        //std::cout << "node kind: " << node->getNodeKind() << std::endl;
        if (node->getNodeKind() == 3)
            continue;
        const PAGNode* pNode = vfg.getLHSTopLevPtr(node);
        const Value* val = pNode->getValue();

        std::vector<const Instruction*> cp;
        std::vector<std::vector<const Instruction*>> ps;
        if (const Function* f = pNode->getFunction()) {
            getCallPaths(f, ps, cp);
        }
        //std::cout << "Value: " << printVal(val) << std::endl;
        int i = 0;
        for (auto p : ps) {
            if (p.empty())
                continue;
            std::cout << "path " << i << ": \n        |--";
            for (auto i : p) {
                std::cout << printVal((Value*)i) << "\n        ---";
            }
            std::cout << (*(p.end() -1))->getFunction()->getName().str() << "|" << std::endl;
            i++;
        }
    }
}


int main(int argc, char ** argv) {
    int arg_num = 0;
    char **arg_value = new char*[argc];
    std::vector<std::string> moduleNameVec;
    SVFUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);

    svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);
    Andersen* ander = AndersenWaveDiff::createAndersenWaveDiff(svfModule);

    PAG* pag = ander->getPAG();
    callgraph = ander->getPTACallGraph();
    callgraph->dump("cg");
    SVFGBuilder svfBuilder;
    SVFG* svfg = svfBuilder.buildFullSVFG(ander);

    for (auto it = svfModule->global_begin(); it != svfModule->global_end(); ++it) {
        llvm::Value* v = *it;
        std::cout << "Value: " << printVal(v) << std::endl;
        collectUsesOnVFG(*svfg, *v);
    }
    /*
    for (auto it = svfModule->llvmFunBegin(); it != svfModule->llvmFunEnd(); ++it) {
        llvm::Function* f = *it;
        std::cout << "operands: " << f->getNumOperands() << std::endl;
        //std::cout << "Function: " << f->getName().str() << std::endl;
        //std::cout << (*it)->getName().str() << std::endl;
        collectUsesOnVFG(*svfg, *f);
    }
    */
}
