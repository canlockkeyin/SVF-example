## 1. clone repository
```
git clone https://github.com/canlockkeyin/SVF-example.git
```

## 2. Install SVF via npm
```
npm install svf-lib
```

You are able to check your installed npm package and its path via command `npm list`
```
$ npm list
/home/samsung
└── svflib@1.0.0
```
Then your_path_to_SVF is `/home/samsung/node_modules/SVF/SVF-linux` or `/home/samsung/node_modules/SVF/SVF-osx`.

your_path_to_LLVM is `/home/samsung/node_modules/llvm-10.0.0.obj`.


## 3. cmake your project by pointing to SVF_DIR and LLVM_DIR
```
cmake -DSVF_DIR=your_path_to_SVF -DLLVM_DIR=your_path_to_LLVM
make
```

## 4. emit IR for the file you want to analyze and run svf-ex on it
```
clang -emit-llvm -S analyze.c -o analyze.ll
./bin/svf-ex analyze.ll
```
