#pragma once

#include "BacktestArguments.hpp"

#include "../opt/Optimizer.hpp"

const string OPTIMIZERS_DIR = fix_path(__DIR__ + "/../opt/optimizers") + "/";

class OptimizeArguments: public BacktestArguments {
public:

    OptimizeArguments(
        int argc, char* argv[], 
        DynLoader& loader,
        bool createIniFilesIfNotExists
    ):
        BacktestArguments(argc, argv, loader)
    {
        addHelp({ "optimizer", "o"}, "Optimizer");

        optimizerLib = OPTIMIZERS_DIR + get<string>("optimizer") + LIB_EXT;
        optimizerIni = OPTIMIZERS_DIR + get<string>("optimizer") + setupExt() + INI_EXT;
        optimizer = loader.load<Optimizer>(optimizerLib);
        optimizer->throwsOnError = true; // TODO: make initializer for shared lib modules
        optimizer->init(optimizerIni, createIniFilesIfNotExists, true);
    }

    virtual ~OptimizeArguments() {}
    
    string getOptimizerLib() const { return optimizerLib; }
    string getOptimizerIni() const { return optimizerIni; }
    Optimizer* getOptimizer() const { return SAFE(optimizer); }

private:

    string optimizerLib;
    string optimizerIni;
    Optimizer* optimizer = nullptr;

};
