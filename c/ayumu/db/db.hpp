//デジタルカーリング
//モンテカルロ関連

namespace DigitalCurling{
    namespace Ayumu{
        /*struct MoveData : public HashData<MoveXY>{
            using base_t = HashData<MoveXY>;
            bool cmp(const MoveData& arg)noexcept{
                if(hash() != arg.hash()){ return true; }
                if((data() >> 48) != (arg.data() >> 48)){ return true; }
                return false;
            }
        };
        
        HashBook<MoveData, (1 << 20)>  moveDB("Move DB");*/
        
        //HashBook<HashData<(uint64_t)>, (1 << 20)>  moveDB("Move DB");
        /*
        void importMoveDB(const std::string& fName){
            RandomAccessor<ShotLog> db;
            readShotLog(fName, &db);
            
            std::mt19937 mt((uint32_t)time(NULL));
            
            moveDB.init();
            
            db.initOrder();
            db.shuffle(mt);
            
            int MIN_RANGE = 3;
            int MAX_RANGE = 3;
            
            int registed = 0;
            
            CERR << "Shot Log : " << db.datas() << " shots were imported." << endl;
            
            iterateRandomly(db, 0, db.datas(), [&](const auto& shot){
                // 最低レンジから順に登録
                // もし前に登録データがあったらレンジを1つ上げる
                ThinkBoard bd;
                bd.setShotLog(shot);
                
                for(int r = MIN_RANGE; r <= MAX_RANGE; ++r){
                    
                    const uint64_t hash = bd.getHash(r);
                    const auto& page = moveDB.page(hash);
                    
                    uint64_t thash = page.hash();
                    MoveXY tmv = page.data();
                    if(tmv.data()){ continue; }
                    
                    MoveXY tmp;
                    convMove(shot.chosenMove, &tmp);
                    
                    tmp.data() |= ((uint64_t)bd.sb()) << 48;
                    tmp.data() |= ((uint64_t)bd.getTurn()) << 44;
                    
                    moveDB.regist(tmp, hash);
                    
                    registed++;
                    
                    return;
                }
            });
            CERR << "Move Data Base : " << registed << " board-move data were registed" << endl;
        }
         */
    }
}