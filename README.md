# sqlike
COP6726 - Database System Implementation - Database From Scratch

## Directory structure
- sqlike/ - project root directory.
    - bin/ - contains bin files generated through `DBFile::Load` method.
        - 1gb/ - contains bin files for 1gb dataset.
        - 10mb/ - contains bin files for 10mb dataset.
    - build/ - contains compiled binaries and executables. This is the directory from where we'll be executing the commands to run our project.
    - docs/ - contains documentation for project description given by the Professor. Also contains screenshots of the results. 
    - files/ - contains tbl dataset files generated through [TPC-H dbgen](https://github.com/electrum/tpch-dbgen). Also contains `catalog` file which holds the schema definitions for our dataset.
        - 1gb/ - contains tbl files for 1gb dataset.
        - 10mb/ - contains tbl files for 10mb dataset. 
    - src/ - contains project source code.
   
Note: The folders `bin`, `build`, and `files/1gb` has not been pushed to GitHub due to storage limitations.

## Project Environment
- OS: Windows with WSL (windows subsystem for linux). I downloaded Ubuntu 20.04LTS from the Microsoft store.
- IDE: CLion with configuration done following [this](https://www.youtube.com/watch?v=xnwoCuHeHuY) and [this](https://github.com/JetBrains/clion-wsl/blob/master/ubuntu_setup_env.sh). I also made some changes in this configuration and provided my setup screenshots(`docs/toolchains-setup.jpg` & `docs/cmake-setup.jpg`) for reference.
- GTest: In addition to this tutorial, I have also configured GTest by running `sudo apt-get install libgtest-dev` inside Ubuntu terminal.

Note: All this configuration commands needs to be run on Ubuntu terminal. I have used `cmake` instead of `make`, so that I can debug using CLion.

## Build Process
Run the following commands in order to run this project on your machine.
- git clone `https://github.com/phoenix-254/sqlike.git`.
- `cd sqlike/src/` - move to src folder.
- `cmake -B../build -H.` - this will generate build folder with all the required files using `CMakeLists.txt` file in `src` folder. (from `src/` directory)
- `cd ../build/` - move to build folder.
- `cmake --build . --target sqlike-test` - compiles the code and generates an executable. (from `build/` directory)

    The `sqlike-test` here is the name of the executable you want to generate, it can be `clean` or any other defined in your `CMakeLists`. e.g in order to clean we can use `cmake --build . --target clean`.
- `./sqlike-test` - to run the code. (from `build/` directory)

Note: You must create empty `bin` folder with two sub-folders(`1gb` & `10mb`) inside the root folder as depicted in above directory structure prior to runnig this project. Also, you have to generate 1gb tbl files using [TPC-H dbgen](https://github.com/electrum/tpch-dbgen) yourself and put it in `files/1gb/` folder if you want to test against 1gb dataset.

## Project Source
- Record: This class implements the actual objects that your database will store and stores all of the data in each record as a flat bit string.
- Page: This is the in-memory realization of a database page; a page is essentially a collection of database records. Previously inside the File class.
- File: This is a disk-based container class that holds an array of pages.
- Comparison: This class implements many of the standard operations that must be provided by the database record manager; that is, they will allow your database to semantically interpret the records that it stores. There is one class called CNF, which is constructed from the parse tree for a conjunctive normal form predicate. This class tells the database system how to apply a user-supplied conjunctive normal form expression to a given records. There is another class called OrderMaker that encodes a less-than/greater-than comparison across two records; this class is used for sorting operations.
- ComparisonEngine: This class contains the code that actually uses the classes that are provided in Comparison.h to perform comparisons. For example, the ne class will allow you to actually use a CNF object to see whether or not a given record has been accepted by the underlying conjunctive normal form predicate.
- Schema: This file encodes a few functions that load up a relation schema from the database catalog using the `catalog` file.
- Config: A simple header file containing static information for the project. e.g Path where tbl or bin files reside.
- Const: A simple header file containing constant values used in this project. Previously Defs.h
- TwoWayList: A data structure used by `Page` to hold collection of records.
- ParseTree: Contains tree structure for the CNF.
- Parser: Used to parse the CNF supplied by the user. This makes it possible for you to easily type CNF statements using the keyboard. This uses Bison library.
- Scanner: Defines rules for how to scan and what action to take for each token given in input CNF by the user. This uses Flex library. Previously Lexer.l.
- DBFile: A driver class that provides an interface for simply storing and retreiving records from the database.

Refer `docs/ProjectDescription.pdf` for more information.
