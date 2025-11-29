void run() {
    gROOT->Macro("loadMuDst.C");

    gSystem->Load("libPhysics");
    gSystem->Load("libGeom");
    gSystem->Load("libVMC");      // <--- Fixes TVirtualMCStack error
    gSystem->Load("libEG");

    gSystem->Load("St_base");
    gSystem->Load("StChain");
    gSystem->Load("StUtilities");
    gSystem->Load("St_Tables");
    gSystem->Load("St_g2t");       // Contains g2t_track_st, g2t_emc_hit_st definitions
    gSystem->Load("StarGeneratorUtil");
    gSystem->Load("StarGeneratorEvent");
    gSystem->Load("StarGeneratorBase");

    // gROOT->LoadMacro("run_smd_test.C+"); 
    
    // gROOT->ProcessLine("run_smd_tests()");
}
