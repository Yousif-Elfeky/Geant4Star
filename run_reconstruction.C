#include "StRoot/macros/bfc.C"
#include "StarSimOpts.h"
#include "StChain/StChain.h"


void run_reconstruction( int nevents=0, const char* jobtag=0, int index=0, std::string dir="./" ) {

    setupProductionJobs();

    std::string inpname;
    StarSimOpt_t job;
    std::string fulltag = jobtag;
    if (fulltag.find("particleGun") != std::string::npos) {
        job = GetStarSimOpts("rcf25000:particleGun");
        std::string cleanName = fulltag;
        std::replace(cleanName.begin(), cleanName.end(), ':', '_');
        std::replace(cleanName.begin(), cleanName.end(), '.', 'p');
        inpname  = cleanName + "_job" + std::to_string(index) + ".geant.root";
        jobtag = "rcf25000:particleGun";
    } 
    else {
        inpname = jobtag;
        std::replace( inpname.begin(), inpname.end(), ':', '_' );
        std::replace( inpname.begin(), inpname.end(), '.', 'p' );
        inpname += "_job";
        inpname += std::to_string(index);
        inpname = inpname + ".geant.root";
    }
    cout << "Input file: " << (dir+inpname) << endl;

    std::string chopts=jobmap[jobtag].recopts;

    bfc(-1,chopts.c_str(),(dir+inpname).c_str());

    gInterpreter->ProcessLine("{Geometry* __hack = new Geometry(); delete __hack;}");

    StMaker *dbMk = chain->GetMaker("db");
    if (dbMk) {
        std::cout << "Setting DB to 'sim' flavor..." << std::endl;
        
        const char* tables[] = {
            "bemcPed", "bemcStatus", "bemcGain", "bemcCalib",
            "bprsPed", "bprsStatus", "bprsGain", "bprsCalib",
            "bsmdePed", "bsmdeStatus", "bsmdeGain", "bsmdeCalib",
            "bsmdpPed", "bsmdpStatus", "bsmdpGain", "bsmdpCalib",
            "eemcPed", "eemcStatus", "eemcGain", "eemcPMTcal",
            ""
        };

        for (int i = 0; tables[i][0] != '\0'; i++) {
            gInterpreter->ProcessLine(Form("((St_db_Maker*)%p)->SetFlavor(\"sim\", \"%s\");", dbMk, tables[i]));
        }
        
    } else {
        std::cout << "WARNING: DB not found!" << std::endl;
    }

    // StMaker *emcSimMk = chain->GetMaker("EmcSimulator");
    // if (emcSimMk) {
    //     std::cout << "Setting StEmcSimulatorMaker debug level to 2..." << std::endl;
    //     gInterpreter->ProcessLine(Form("((StEmcSimulatorMaker*)%p)->SetDebug(2);", emcSimMk));
    // } else {
    //     std::cout << "WARNING: StEmcSimulatorMaker not found!" << std::endl;
    // }


    // StMaker *geantMk = chain->GetMaker("geant");
    // if (geantMk) {
    //     std::cout << "Configuring St_geant_Maker for AgML Legacy Support..." << std::endl;
        
    //     // 1. Tell it we are NOT running a full Geant3 simulation
    //     // gInterpreter->ProcessLine(Form("((St_geant_Maker*)%p)->SetNwGEANT(0);", geantMk));

    //     // 2. Load the Geometry explicitly using AgML syntax
    //     // This triggers the creation of .const/geom/calb_calg
    //     // gInterpreter->ProcessLine(Form("((St_geant_Maker*)%p)->LoadGeometry(\"detp geometry y2024a\");", geantMk));
        

    //     // geantMk->SetActive(kTRUE);
    // } else {
    //     std::cout << "ERROR: 'geant' maker not found! You must add 'geant' to recopts." << std::endl;
    // }

    chain->Init();

    chain->EventLoop(nevents,chain->Maker("outputStream"));

    std::cout << jobtag << std::endl;
    std::cout << jobmap[jobtag].recopts.c_str() << std::endl;
    std::cout << inpname.c_str() << std::endl;

}
