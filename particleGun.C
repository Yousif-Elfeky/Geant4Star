class StBFChain;        
class StMessMgr;

#include "StarSimOpts.h"
#include <string>
#include <TString.h>

#include "StRoot/macros/bfc.C"
#include "StarSimOpts.h"
#include "StChain/StChain.h"

#pragma cling load("StarRoot")
#pragma cling load("St_base")
#pragma cling load("StChain")
#pragma cling load("StUtilities")
#pragma cling load("StBFChain")
#pragma cling load("liblog4cxx")
#pragma cling load("StStarLogger.so")
#pragma cling load("StarClassLibrary.so")
#pragma cling load("libmysqlclient.so")
#pragma cling load("libStarMiniCern.so")

#if !(defined(__CINT__) || defined(__CLING__)) || defined(__MAKECINT__)


#include "Stiostream.h"
#include "TSystem.h"
#include "TClassTable.h"
#include "TApplication.h"
#include "TInterpreter.h"
#include "StBFChain.h"
#include "StMessMgr.h"
#include "TROOT.h"
#include "TAttr.h"
#include "Rtypes.h"

#endif


// Defined w/in the StBFChain library
extern StBFChain* chain;

StBFChain* top = new StBFChain("physicssim");

// Minimal setup to load libraries
std::string chainopts="nodefault y2024a agml stargen:mk kinematics:mk g4star:mk noinput geant4out ";

StMaker* g4star = 0;

void particleGun( 
    int nevents=100, 
    int ntracks=20,
    std::string fulltag="rcf25000:particleGun:pid3:pt5.0to20.0:eta-1.0to1.0",
    const int index=0
    )
{
        
    std::string physlist="FTFP_BERT";	  
    std::string distribution="FlatPT";

    setupProductionJobs();


    int pid = 11; 
    double ptmn=0, ptmx=0, etamn=0, etamx=0;
    double phimn=0, phimx=2*TMath::Pi();
    
    // We use ROOT's TString for parsing because why not
    TString tStr = fulltag;
    TObjArray* tokens = tStr.Tokenize(":");
    
    for(int i=0; i<tokens->GetEntries(); i++) {
        TString token = ((TObjString*)tokens->At(i))->String();
        if (token.BeginsWith("pid")) {
            sscanf(token.Data(), "pid%d", &pid);
        }
        else if (token.BeginsWith("pt")) {
            sscanf(token.Data(), "pt%lfto%lf", &ptmn, &ptmx);
        }
        else if (token.BeginsWith("eta")) {
            sscanf(token.Data(), "eta%lfto%lf", &etamn, &etamx);
        }
    }
    
    std::cout << "--- DECODED PARAMETERS ---" << std::endl;
    std::cout << "PID: " << pid << " | Pt: " << ptmn << " to " << ptmx << " | Eta: " << etamn << " to " << etamx << std::endl;

    auto job = GetStarSimOpts("rcf25000:particleGun");

    std::string outname = fulltag;
    std::replace(outname.begin(), outname.end(), ':', '_');
    std::replace(outname.begin(), outname.end(), '.', 'p');
    outname += "_job" + std::to_string(index) + ".geant.root";

    chainopts = job.simopts;
    chainopts += " ";
    chainopts += job.timestamps[0];

    std::cout << "chain: " << job.simopts << std::endl;
    std::cout << "name:  " << job.name << std::endl;
    std::cout << "out:   " << outname << std::endl;

    top->SetDebug(1);
    top->SetFlags(chainopts.c_str());
    top->Set_IO_Files(0, outname.c_str());
    top->Load();
    top->Instantiate();
    top->SetAttr(".RunNumber", job.runnumbers[0]);
    auto* prim = top->Maker("PrimaryMaker");
    auto* kine = top->Maker("StarKine");

    //
    // Geant4STAR configuration
    //
    //  const char* g4commands =
    //    "/process/eLoss/maxKinEnergy 1000.0 GeV" 
    //    ;
    g4star = top->Maker("geant4star");
    g4star->SetAttr("G4VmcOpt",physlist.c_str());

    g4star->SetAttr(
        "G4UI:PREINIT", 
        "/process/eLoss/maxKinEnergy 250.0 GeV"   ";" 
        "/mcCrossSection/setMaxKinE  250.0 GeV"   ";"
    );
    g4star->SetAttr(
        "G4UI:INIT",
        "/mcCrossSection/setMinKinE 1 keV "       ";"
        "/mcCrossSection/setMaxKinE 250 GeV "     ";"
        "/mcCrossSection/setMinMomentum 10 keV "  ";"
        "/mcCrossSection/setMaxMomentum 250 GeV " ";"
    );
    g4star->SetAttr("runnumber", job.runnumbers[0] );

    // g4star->SetAttr(
    //      "G4UI:POSTTRIG",
    //      "/mcPhysics/printGlobalCuts"
    // );

    //  g4star->SetAttr("G4UI:INIT",    "interactive"  );

    //  g4star -> SetAttr("G4VmcOpt:Phys",  "FTFP_BERT");
    //  g4star -> SetAttr("G4VmcOpt:Phys",  "QGSP_BERT");
    
    
    prim->AddMaker( kine );
    prim->SetAttr("verbose",111);
    for ( const auto dattr : job.primDAttr ) { prim->SetAttr( dattr.first.c_str(), dattr.second );  } // defaults

    kine->SetAttr("mode",    distribution.c_str() );
    kine->SetAttr("ptlow",   ptmn     );
    kine->SetAttr("pthigh",  ptmx     );
    kine->SetAttr("etalow",  etamn    );
    kine->SetAttr("etahigh", etamx    );
    kine->SetAttr("phimn",   phimn    );
    kine->SetAttr("phimx",   phimx    );
    kine->SetAttr("pid",     pid      );
    kine->SetAttr("ntrack",  ntracks  );

    top->ls(5);
    gInterpreter->ProcessLine("{Geometry* __hack = new Geometry(); delete __hack;}");

    top->Init();

    gSystem->SetFPEMask( kNoneMask );
    top->EventLoop(nevents, top->Maker("outputStream"));
    top->Finish();

};



