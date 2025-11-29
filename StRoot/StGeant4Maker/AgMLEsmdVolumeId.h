#ifndef __AgMLEsmdVolumeId_h__
#define __AgMLEsmdVolumeId_h__

#include <StarVMC/StarAgmlLib/AgMLExtension.h>
#include <TVirtualMC.h>
#include <cstring>
#include <iostream>
#include <iomanip>

class AgMLEsmdVolumeId : public AgMLVolumeId {

private:
  
  // Wheel/Sector Map: Maps (WheelIndex, SectorIndex) -> PhysicalSector(1-12)
  const int sectormap[2][6] = { 
    { 4, 5, 6, 7, 8, 9}, // Wheel 1 (Bottom/Right)
    {10,11,12, 1, 2, 3}  // Wheel 2 (Top/Left)
  };

  // Section Type Map: Maps (SectionIndex, LocalPhi) -> Type
  // Key: 1=V, 2=U, 3=cutV, 4=cutU, 0=Empty
  // Row 0 = ESPL_1, Row 1 = ESPL_2, Row 2 = ESPL_3
  // check the EcalGeo6.xml and the geom.root
  const int sectionTypes[3][6] = {
    {4, 1, 0, 2, 1, 0}, // Section 1
    {0, 2, 1, 0, 2, 3}, // Section 2
    {1, 0, 2, 1, 0, 2}  // Section 3
  };

public:
  
  AgMLEsmdVolumeId() : AgMLVolumeId() { };

  virtual int id( int* numbv ) const { 
    // numbv: [0]=Wheel, [1]=SectorIdx, ... [N]=Strip
    
    int wheel   = numbv[0]; 
    int sec_idx = numbv[1];
    
    // Find Strip ID (Deepest non-zero index)
    int strip = 0;
    if      (numbv[5] > 0) strip = numbv[5];
    else if (numbv[4] > 0) strip = numbv[4];
    else if (numbv[3] > 0) strip = numbv[3];
    else                   strip = numbv[2];

    int section = 1; // Default
    
    // Physical Sector (1-12)
    int w_idx = (wheel >= 1 && wheel <= 2) ? wheel - 1 : 0;
    int s_idx = (sec_idx >= 1 && sec_idx <= 6) ? sec_idx - 1 : 0;
    int sector = sectormap[w_idx][s_idx];

    // Half (1 or 2)
    int half = (sector >= 4 && sector <= 9) ? 1 : 2;

    // Local Phi (1-6)
    // This maps the 12 global sectors into the 6 slots of the lookup table
    int local_phi = 0;
    if (half == 1) {
        local_phi = sector - 3; // 4->1, 5->2 ... 9->6
    } else {
        if (sector >= 10) local_phi = sector - 9; // 10->1, 11->2, 12->3
        else local_phi = sector + 3;              // 1->4, 2->5, 3->6
    }
    
    // magic shit for the array index (needs review)
    int phi_idx = (local_phi >= 1 && local_phi <= 6) ? local_phi - 1 : 0;
    int sect_idx = (section >= 1 && section <= 3) ? section - 1 : 0;

    //get the type (u,v)    
    int typeCode = sectionTypes[sect_idx][phi_idx];
    
    bool isU = (typeCode == 2 || typeCode == 4);
    bool isV = (typeCode == 1 || typeCode == 3);
    
    // If we hit a "0", treat as U to avoid ID corruption,
    // though this shouldn't happen at the first place
    // since the geant4maker checks the number of hits and
    // if we do get it the slow simulator will remove it
    // for some reason getting more hits in smdv than smdu??
    if (!isU && !isV) isU = true; 

    // more magic
    // copied from the getvolidEsmd
    // somehow works, don't know why
    int plane_code = 0;

    if ( local_phi == 1 || local_phi == 4 ) {
        plane_code = isU ? 1 : 3;
    } 
    else if ( local_phi == 2 || local_phi == 5 ) {
        plane_code = isU ? 2 : 1; 
    } 
    else if ( local_phi == 3 || local_phi == 6 ) {
        plane_code = isU ? 3 : 2;
    }
    int volumeid =  (half       * 1000000) //ocd kicked in
                 +  (local_phi  * 10000  ) //looks nice though
                 +  (plane_code * 1000   ) 
                 +  (strip               );

    return volumeid;
  };
};

#endif