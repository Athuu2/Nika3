#pragma once
class Random
{
private:
    ConfigLoader* cl;
    MyDisplay* display;
    Level* map;
    LocalPlayer* lp;
    std::vector<Player*>* players;
public:
    Random(ConfigLoader* configLoada,
          MyDisplay* myDisplay,
          Level* level,
          LocalPlayer* localPlayer,
          std::vector<Player*>* all_players
          )
    {
        cl = configLoada;
        display = myDisplay;
        map = level;
        lp = localPlayer;
        players = all_players;
    }
    int tmpSpectator = 0;
    //==========---------[SUPERGLIDE]-------------==================
    void superGlide(int counter){
        while (display->keyDown(XK_space))
        {
            static float startjumpTime = 0;
            static bool startSg = false;
            static float traversalProgressTmp = 0.0;
 
            float worldtime = mem::Read<float>(lp->base + OFFSET_TIME_BASE); // Current time
            float traversalStartTime = mem::Read<float>(lp->base + OFFSET_TRAVERSAL_START_TIME); // Time to start wall climbing
            float traversalProgress = mem::Read<float>(lp->base + OFFSET_TRAVERSAL_PROGRESS); // Wall climbing, if > 0.87 it is almost over.
            auto HangOnWall = -(traversalStartTime - worldtime);
 
            if (HangOnWall > 0.1 && HangOnWall < 0.24)
            {
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
            }
            if (traversalProgress > 0.87f && !startSg && HangOnWall > 0.05f && HangOnWall < 1.5f)
            {
                //start SG
                startjumpTime = worldtime;
                startSg = true;
            }
            if (startSg)
            {
                //printf ("sg Press jump\n");
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                while (mem::Read<float>(lp->base + OFFSET_TIME_BASE) - startjumpTime < 0.011);
                {
                    mem::Write<int>(OFF_REGION + OFF_IN_DUCK + 0x8, 6);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
                    std::this_thread::sleep_for(std::chrono::milliseconds(600));
                }
                startSg = false;
                break;
            }
        }
 
        // Automatic wall jump
        int wallJumpNow = 0;
 
        static float onWallTmp = 0;
        float onWall = mem::Read<float>(lp->base + OFFSET_WALL_RUN_START_TIME);
        if (onWall > onWallTmp + 0.1) // 0.1
        {
            if (mem::Read<int>(OFF_REGION + OFFSET_IN_FORWARD) == 0)
            {
                wallJumpNow = 1;
                //printf("wall jump Press jump\n");
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
            }
        }
        onWallTmp = onWall;
 
        static float onEdgeTmp = 0;
        float onEdge = mem::Read<float>(lp->base + OFFSET_TRAVERSAL_PROGRESS);
        if (onEdge > onEdgeTmp + 0.1) // 0.1
        {
            if (mem::Read<int>(OFF_REGION + OFFSET_IN_FORWARD) == 0)
            {
                wallJumpNow = 2;
                //printf("wall jump onEdge Press jump\n");
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
            }
        }
        onEdgeTmp = onEdge;    
    }


    //==========---------[QUICKTURN]-------------==================
    void quickTurn(int counter)
    {
        Vector2D localYawtoClamp = lp->viewAngles;
        localYawtoClamp.Clamp();
        float localYaw = localYawtoClamp.y;
        // quickTurn
        if(cl->FEATURE_QUICKTURN_ON){
            if(display->keyDown(cl->FEATURE_QUICKTURN_BUTTON)){
                lp->setYaw((localYaw + 180));
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    }

    //====================---------[PRINTLEVELS]-------------------------==================
    void MapRadar(int counter) {
        if (display->keyDown(cl->FEATURE_MAP_RADAR_BUTTON) && cl->FEATURE_MAP_RADAR_ON) {
            uint64_t pLocal = mem::Read<uint64_t>(OFF_REGION + OFF_LOCAL_PLAYER);

            int currentTEAM = mem::Read<int>(pLocal + OFF_TEAM_NUMBER);
            
            for (uintptr_t i = 0; i <= 80000; i++)
            {
            mem::Write<int>(pLocal + OFF_TEAM_NUMBER, 1);
            }
            for (uintptr_t i = 0; i <= 80000; i++)
            {
            mem::Write<int>(pLocal + OFF_TEAM_NUMBER, currentTEAM);
            } 
        }
    }

    //====================---------[PRINTLEVELS]-------------------------==================
    void printLevels(int counter)
    {
        if(cl->FEATURE_PRINT_LEVELS_ON){

            if(display->keyDown(cl->FEATURE_PRINT_LEVELS_BUTTON)){
                printf("[N]=[NAME]-[LEVEL]-[LEGEND]\n\n");
                for (auto i = 0; i < players->size(); i++)
                {
                    Player *p = players->at(i);
                    if(!p->dead && p->isPlayer()){
                        int playerLvl = p->GetPlayerLevel();
                        std::string namePlayer = p->getPlayerName();
                        std::string modelName = p->getPlayerModelName();
                        
                        if(p->friendly){
                            printf("\033[91m[%i]=[%s]-[%i]-[%s]\033[0m\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());  
                        } else if (playerLvl > 900 &&  playerLvl < 1900){
                            printf("\033[94m[%i]=[%s]-[%i]-[%s]\033[0m\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());
                        } else if (playerLvl > 1900){
                            printf("\033[92m[%i]=[%s]-[%i]-[%s]\033[0m\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());
                        } else {
                            printf("[%i]=[%s]-[%i]-[%s]\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());
                        }
                    }           
                }   
            }            
        }        
    }

    //==================---------------[SPECTATORVIEW]-------------------==================
    void spectatorView(int counter)
    {
        if(!map->playable) return;
        int spectatorcount = 0;   
        std::vector<std::string> spectatorlist;
        if(cl->FEATURE_SPECTATOR_ON){
            for (int i = 0; i < players->size(); i++)
            { 
                Player *p = players->at(i);          
                      
                float targetyaw = p->view_yaw; // get Yaw player
                float localyaw= lp->local_yaw; // get Yaw LocalPlayer
                //printf("LocalViewYaw: %f == PlayerViewYaw: %f \n", localyaw, targetyaw);
                if (targetyaw == localyaw && p->currentHealth == 0){
                    spectatorcount++;
                    tmpSpectator = spectatorcount;
                    
                    std::string namePlayer = p->getPlayerName();    
                    spectatorlist.push_back(namePlayer);
                }            
            }
            const auto spectatorlist_size = static_cast<int>(spectatorlist.size());
           
            if (spectatorcount > 0){
                printf("\n-[%d]-- SPECTATORS -- \n", spectatorcount);
                for (int i = 0; i < spectatorlist_size; i++)   //show list of spectators by name
                {   
                    printf("---[%s]---\n", spectatorlist.at(i).c_str());
                }
            }              
        }      
    }

    //==================---------[SKINCHANGER]-------------==========================================
    void SkinChange(int counter)
    {
        if(!map->playable) return;
        if(lp->dead) return;
        float curTime = mem::Read<float>(lp->base + OFFSET_TIME_BASE);
        float endTime = curTime +5.5;
        std::map<int, std::vector<int>> weaponSkinMap;
        //Light ammo weapons
        weaponSkinMap[105] = { 6 };   //WEAPON_P2020 
        weaponSkinMap[81] = { 6 };   //WEAPON_RE45 
        weaponSkinMap[80] = { 11 };   //WEAPON_ALTERNATOR 
        weaponSkinMap[104] = { 2 };   //WEAPON_R99  
        weaponSkinMap[0] = { 10 };     //WEAPON_R301   
        weaponSkinMap[106] = { 2 };    //WEAPON_SPITFIRE 
        weaponSkinMap[89] = { 5 };    //WEAPON_G7 
        //Heavy ammo weapons
        weaponSkinMap[112] = { 10};   // Car-SMG 
        weaponSkinMap[21] = { 6 };    // Rampage 
        weaponSkinMap[111] = { 9 };      //3030 
        weaponSkinMap[90] = {10 };   //WEAPON_HEMLOCK  
        weaponSkinMap[88] = { 8 };    //FlatLine  
        //Energy ammo weapons
        weaponSkinMap[113] = { 8 };    //WEAPON_NEMESIS  
        weaponSkinMap[110] = { 9 };    //WEAPON_VOLT 
        weaponSkinMap[107] = { 7 };    //WEAPON_TRIPLE_TAKE 
        weaponSkinMap[93] = { 3 };    //WEAPON_LSTAR 
        weaponSkinMap[84] = { 5 };    //WEAPON_DEVOTION 
        weaponSkinMap[86] = { 8 };    //WEAPON_HAVOC 
        //Sniper ammo weapons
        weaponSkinMap[1] = { 5 };    //WEAPON_SENTINEL 
        weaponSkinMap[83] = { 8 };    //WEAPON_CHARGE_RIFLE 
        weaponSkinMap[85] = { 7 };    //WEAPON_LONGBOW 
        //Shotgun ammo weapons
        weaponSkinMap[96] = { 5 };    //WEAPON_MOZAMBIQUE 
        weaponSkinMap[87] = { 8 };    //WEAPON_EVA8 
        weaponSkinMap[103] = { 7 };    //WEAPON_PEACEKEEPER 
        weaponSkinMap[95] = { 5 };    //WEAPON_MASTIFF 
        //Legendary ammo weapons
        weaponSkinMap[109] = { 5 };    //WEAPON_WINGMAN 
        weaponSkinMap[102] = { 7 };    //WEAPON_PROWLER
        weaponSkinMap[2] = { 3 };    //WEAPON_BOCEK
        weaponSkinMap[92] = { 6 };    //WEAPON_KRABER
        weaponSkinMap[163] = { 3 };    //WEAPON_THROWING_KNIFE
        weaponSkinMap[164] = { 2 };    //WEAPON_THERMITE_GRENADE 
        weaponSkinMap[3] = { 2 };    //WEAPON_BUSTER_SWORD_R25 

        if (cl->FEATURE_SKINCHANGER_ON){
            int waponIndex = mem::Read<int>(lp->weaponEntity + OFF_WEAPON_INDEX);
            if (weaponSkinMap.count(waponIndex) == 0) return;
            int skinID = weaponSkinMap[waponIndex][0];
            //printf("Weapon: %s Activated Skin ID: %d \n", WeaponName(waponIndex).c_str(), skinID);  
            mem::Write<int>(lp->base + OFF_SKIN, skinID+1);
            mem::Write<int>(lp->weaponEntity + OFF_SKIN, skinID);
        }                    
    }
};
