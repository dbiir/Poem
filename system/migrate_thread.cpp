#include "migrate_thread.h"
#include "migmsg_queue.h"

#include "global.h"
#include "helper.h"
#include "manager.h"
#include "thread.h"
#include "io_thread.h"
#include "query.h"
#include "ycsb_query.h"
#include "tpcc_query.h"
#include "mem_alloc.h"
#include "transport.h"
#include "math.h"
#include "msg_thread.h"
#include "msg_queue.h"
#include "message.h"
#include "client_txn.h"
#include "work_queue.h"
#include "txn.h"
#include "wl.h"
#include "ycsb.h"
#include "tpcc.h"
<<<<<<< HEAD
=======
#include "tpcc_helper.h"
#include "tpcc_const.h"
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
#include "index_btree.h"
#include "index_hash.h"
#include "table.h"
#include "wl.h"

void MigrateThread::setup() {
    /*
    std::vector<Message*> * msgs;
	while(!simulation->is_setup_done()) {
		msgs = tport_man.recv_msg(get_thd_id());
		if (msgs == NULL) continue;
		while(!msgs->empty()) {
			Message * msg = msgs->front();
			if(msg->rtype == INIT_DONE) {
				printf("Received INIT_DONE from node %ld\n",msg->return_node_id);
				fflush(stdout);
				simulation->process_setup_msg();
			} else {
				assert(ISSERVER || ISREPLICA);
				//printf("Received Msg %d from node %ld\n",msg->rtype,msg->return_node_id);
                #if CC_ALG == CALVIN
			        if(msg->rtype == CALVIN_ACK ||(msg->rtype == CL_QRY && ISCLIENTN(msg->get_return_id())) ||
				    (msg->rtype == CL_QRY_O && ISCLIENTN(msg->get_return_id()))) {
				    work_queue.sequencer_enqueue(get_thd_id(),msg);
				    msgs->erase(msgs->begin());
				    continue;
			    }
			    if( msg->rtype == RDONE || msg->rtype == CL_QRY || msg->rtype == CL_QRY_O) {
				    assert(ISSERVERN(msg->get_return_id()));
				    work_queue.sched_enqueue(get_thd_id(),msg);
				    msgs->erase(msgs->begin());
				    continue;
			    }
                #endif
				work_queue.enqueue(get_thd_id(),msg,false);
			}
			msgs->erase(msgs->begin());
		}
		delete msgs;
	}
    */
	if (!ISCLIENT) {
		txn_man = (YCSBTxnManager *)
		    mem_allocator.align_alloc( sizeof(YCSBTxnManager));
		new(txn_man) YCSBTxnManager();
		// txn_man = (TxnManager*) malloc(sizeof(TxnManager));
		uint64_t thd_id = get_thd_id();
		txn_man->init(thd_id, _wl);
        txn_man->h_wl = (YCSBWorkload*)_wl;
	}    
}

TxnManager * MigrateThread::get_transaction_manager(MigrationMessage * msg) {
    #if CC_ALG == CALVIN
        TxnManager* local_txn_man =
        txn_table.get_transaction_manager(get_thd_id(), msg->get_txn_id(), msg->get_batch_id());
    #else
        TxnManager * local_txn_man = txn_table.get_transaction_manager(get_thd_id(),msg->get_txn_id(),0);
    #endif
    return local_txn_man;
}

RC MigrateThread::run(){
    tsetup();
    printf("Running MigrateThread %ld\n",_thd_id);

    //uint64_t ready_starttime;
    //uint64_t idle_starttime = 0;

    while(!simulation->is_done()) {
        txn_man = NULL;
        heartbeat();
        progress_stats();
<<<<<<< HEAD
        MigrationMessage* msg = new(MigrationMessage);
=======
        MigrationMessage* msg = new (MigrationMessage);
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
        //printf("begin receive message!\n");
        while(migmsg_queue.get_size()==0){
            if (simulation->is_done()) return RCOK;
        }
        //std::cout<<"the size is "<<migmsg_queue.get_size()<<endl;
        uint64_t dest = migmsg_queue.dequeue(get_thd_id(),msg);
        //std::cout<<"the size is "<<migmsg_queue.get_size()<<endl;

        assert(dest>=0);
<<<<<<< HEAD
        //printf("get message111!\n");
        //if (msg->node_id_src != g_node_id) continue;
        //printf("get message!\n");
=======
        //printf("get message!\n");
        //std::cout<<"Time:"<<(get_sys_clock() - g_starttime) / BILLION<<endl;
        //std::cout<<"node_id_des is "<<msg->node_id_des<<endl;
        //std::cout<<"message type is:"<<msg->get_rtype()<<" by migrate dequeue"<<endl;
        //std::cout<<"the size of msg is "<<msg->get_size()<<endl;
    
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
        //std::cout<<"node_id_des is "<<msg->node_id_des<<endl;
        //std::cout<<"message type is:"<<msg->get_rtype()<<" by migrate dequeue"<<endl;
        RC rc;
        txn_man = get_transaction_manager(msg);
        txn_man->register_thread(this);
        txn_man->h_wl = _wl;
        switch(msg->get_rtype()){
            case SEND_MIGRATION:
                rc = process_send_migration(msg);
                assert(rc==RCOK);
                break;
            case RECV_MIGRATION:
                rc = process_recv_migration(msg);
                assert(rc==RCOK);
                break;
            case FINISH_MIGRATION:
                rc = process_finish_migration(msg);
                assert(rc==RCOK);
                break;
            default:
                printf("message->rtpye is %d\n",msg->get_rtype());
                break;
        }
        delete msg;
    }
    return RCOK;
}

RC MigrateThread::process_send_migration(MigrationMessage* msg){
#if WORKLOAD == YCSB
    DEBUG("SEND_MIGRATION %ld\n",msg->get_txn_id());
<<<<<<< HEAD
    std::cout<<"SEND_MIGRATION Time:"<<(get_sys_clock() - g_start_time) / BILLION<<endl;
    //if (g_mig_starttime == 0) g_mig_starttime = get_sys_clock();
    g_mig_tmp_time = get_sys_clock();

    //debug
    //for (uint i=0; i< msg->mig_order.size(); i++) std::cout<<msg->mig_order[i]<<' ';    


=======
    std::cout<<"SEND_MIGRATION Time:"<<(get_sys_clock() - g_starttime) / BILLION<<endl;
    if (g_mig_starttime == 0) g_mig_starttime = get_sys_clock();
    std::cout<<&msg<<endl;
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
    RC rc =RCOK;
    start_time = get_sys_clock();
    txn_man->return_id = ((MigrationMessage*)msg)->return_node_id;
    txn_man->h_wl = _wl;
    update_part_map_status(msg->part_id, 1); //migrating
    #if (MIGRATION_ALG == DETEST)
        update_detest_status(1);
        update_minipart_map_status(msg->minipart_id, 1);
        #if REMUS_SPLIT
            update_part_map_status(msg->part_id, 1);
            if (remus_status == 0){
                update_remus_status(1);
            }
            else {
                std::cout<<"%%%%%%%%%%remus != 0"<<endl;
            }
        #endif
    #elif (MIGRATION_ALG == DETEST_SPLIT)
        if (detest_status == 0){
            update_detest_status(1);
        }
        update_row_map_status_order(msg->order,1);
    #elif (MIGRATION_ALG == REMUS)
        update_remus_status(1);
        update_part_map_status(msg->part_id, 1);
    #elif (MIGRATION_ALG == SQUALL)
        update_squall_status(1);
        update_squallpart_map_status(msg->minipart_id, 1);
        update_part_map_status(msg->part_id, 1);
    #endif
    
    
<<<<<<< HEAD
    ((MigrationMessage*)msg)->isdata = false;
    //std::cout<<&msg<<endl;
    std::cout<<"the size of msg is "<<((MigrationMessage*)msg)->get_size()<<endl;

    //update the migration metadata
    #if MIGRATION_ALG==DETEST
        update_part_map_status(((MigrationMessage*)msg)->part_id, 1);
        update_minipart_map_status(
            ((MigrationMessage*)msg)->part_id,
            ((MigrationMessage*)msg)->minipart_id, 1);
    #endif

    //init metadata
    uint64_t key_ptr = ((MigrationMessage*)msg)->key_start;//scan ptr
    uint64_t msg_num = 0;//msg max num
    msg_num = max(MSG_CHUNK_SIZE / MAX_TUPLE_SIZE - 15, 1);
    std::cout<<"msg_num is "<<msg_num<<endl;
    
    bool new_msg = true;//whether to construct new msg
    MigrationMessage *msg1;

    //fetch data and add into msg
    while(key_ptr <= ((MigrationMessage*)msg)->key_end){
        //construct new msg
        if (new_msg){
            msg1 = new MigrationMessage;
            *msg1 = *msg;  
            msg1->isdata = true;  
            msg1->islast = false;
            msg1->data_size = 0;
            msg1->key_start = key_ptr; 
            new_msg = false;
        }


        itemid_t* item = new(itemid_t);
        RC rc=WAIT;
        while (rc != RCOK){
            rc = ((YCSBWorkload*)_wl)->the_index->index_read(key_ptr,item,key_to_part(key_ptr),g_thread_cnt); 
        }

        row_t* row = new(row_t);
        row = ((row_t*)item->location);
        row_t* row_rtn = new(row_t);
        access_t access;
        #if MIGRATION_ALG == DETEST
            access = access_t::RD;
        #endif
        rc = txn_man->get_row(row,access,row_rtn);
        if (rc != RCOK){
            std::cout<<"trying to get lock..."<<endl;
        }
        while(rc != RCOK){
            rc = txn_man->get_row(row,access,row_rtn);
        }        

        msg1->data.emplace_back(*row_rtn);
        char tmp_char[MAX_TUPLE_SIZE];
        strcpy(tmp_char, row_rtn->get_data());
        string tmp_str = tmp_char;
        msg1->row_data.emplace_back(tmp_str);
        msg1->data_size ++;


        //send this msg
        if (msg1->data_size >= msg_num || key_ptr == ((MigrationMessage*)msg)->key_end){
            msg1->return_node_id = g_node_id;
            msg1->rtype = RECV_MIGRATION;
            msg1->key_end = key_ptr;
            if (key_ptr == ((MigrationMessage*)msg)->key_end){
                msg1->islast = true;
            }

            //debug
            //for (uint i=0; i< msg1->mig_order.size(); i++) std::cout<<msg1->mig_order[i]<<' ';              

            msg_queue.enqueue(get_thd_id(), msg1, msg1->node_id_des);

            std::cout<<"key up to "<<key_ptr<<endl;

            if (key_ptr == ((MigrationMessage*)msg)->key_end) break;

            new_msg = true;

        }

        key_ptr += PART_CNT;
    }

    std::cout<<"Send migration from "<<msg1->node_id_src<<" to "<<msg1->node_id_des<<" part_id "<<msg1->part_id<<" minipart_id "<<msg1->minipart_id<<endl;

    //update migration metadata
    #if MIGRATION_ALG == DETEST
        update_minipart_map_status(
            ((MigrationMessage*)msg)->part_id,
            ((MigrationMessage*)msg)->minipart_id, 1);
        update_part_map_status(((MigrationMessage*)msg)->part_id, 1);
    #endif    

=======
    msg->isdata = false;
    #if MIGRATION_ALG == DETEST_SPLIT
        idx_key_t key;
        uint64_t label = Order[msg->order]; //这次迁移哪一类的row
        for (uint64_t i=0;i<order_map[label].size();i++){ //遍历这一类的row
            key = order_map[label][i];
            itemid_t* item1 = (itemid_t*)mem_allocator.alloc(sizeof(itemid_t));
	        itemid_t* &item = item1;
            RC rc = ((YCSBWorkload*)_wl)->the_index->index_read(key,item,key_to_part(key),g_thread_cnt);
            row_t* row = ((row_t*)item->location);
            row_t* row_rtn = new(row_t);
            access_t access;
            access = access_t::RD;
            rc = txn_man->get_row(row,access,row_rtn);
            if (rc != RCOK){
                std::cout<<"trying to get lock..."<<endl;
            }
            while(rc != RCOK){
                rc = txn_man->get_row(row,access,row_rtn);
            }
        }
    #else
        uint64_t key = ((MigrationMessage*)msg)->key_start;
        std::cout<<"key is "<<key<<endl;
        std::cout<<msg->data_size<<endl;
        if (msg->data_size * MAX_TUPLE_SIZE > (MSG_CHUNK_SIZE - 500)){
            uint64_t tuple_num = max(MSG_CHUNK_SIZE / MAX_TUPLE_SIZE - 15, 1); //每个消息传递的tuple数量
            uint64_t msg_num; //要传递的次数
            msg_num = msg->data_size / tuple_num + 1; 
            
            for (uint i=0;i<msg_num;i++){
                MigrationMessage* msg1 = new(MigrationMessage);
                *msg1 = *msg;
                if (i != msg_num-1) {
                    msg1->data_size = tuple_num;
                    msg1->islast = false;
                }
                else {
                    msg1->data_size = msg->data_size - (msg_num -1)*tuple_num;
                    msg1->islast = true;
                }
                msg1->key_start = i * tuple_num + msg->key_start;
                std::cout<<"msg1->key_start is "<<msg1->key_start<<endl;

                //int64_t starttime,endtime;
                //starttime = get_sys_clock();
                for (uint64_t j=0;j<msg1->data_size;j++){
                    itemid_t* item = new(itemid_t);
                    RC rc=WAIT;
                    while (rc != RCOK){
                        rc = ((YCSBWorkload*)_wl)->the_index->index_read(key,item,key_to_part(key),g_thread_cnt); 
                    }
                    //std::cout<<"index_read!"<<endl;

                    row_t* row = new(row_t);
                    row = ((row_t*)item->location);
                    row_t* row_rtn = new(row_t);
                    access_t access;
                    #if (MIGRATION_ALG == REMUS )
                        access = access_t::WR;
                    #elif (MIGRATION_ALG == DETEST)
                        access = access_t::WR;
                    #elif (MIGRATION_ALG == DETEST_SPLIT)
                        access = access_t::RD;
                    #else
                        access = access_t::WR;
                    #endif
                    #if CC_ALG == NO_WAIT || CC_ALG==WAIT_DIE || CC_ALG == DL_DELETE
                    access = access_t::WR;
                    #endif
                    rc = txn_man->get_row(row,access,row_rtn);
                    if (rc != RCOK){
                        std::cout<<"trying to get lock..."<<endl;
                    }
                    while(rc != RCOK){
                        rc = txn_man->get_row(row,access,row_rtn);
                    }
                    //std::cout<<"get_row!"<<endl;

                    msg1->isdata=true;
                    msg1->data.emplace_back(*row_rtn);
                    
                    char tmp_char[MAX_TUPLE_SIZE];
                    strcpy(tmp_char, row_rtn->get_data());
                    string tmp_str = tmp_char;

                    msg1->row_data.emplace_back(tmp_str);
                    
                    //std::cout<<"key is "<<msg1->data[i].get_primary_key();
                    //std::cout<<"data is "<<msg1->row_data[i]<<endl;
                    
                    if (KEY_TO_PART == HASH_MODE) key = key + PART_CNT;
                    else key ++;
                }
                //endtime = get_sys_clock();
                //std::cout<<"read data end! Time:"<<(endtime-starttime) / BILLION<<endl;
                //printf("the size of msg row is %ld\n",msg1->data_size);
                //std::cout<<"the size of msg is "<<msg1->get_size()<<endl;

                if(rc != WAIT) {
                    msg1->return_node_id = g_node_id;
                    msg1->rtype = RECV_MIGRATION;
                    msg_queue.enqueue(get_thd_id(), msg1, msg1->node_id_des);
                    std::cout<<"enqueue finished "<<i<<endl;
                    std::cout<<"msg1->node_id_des is "<<msg1->node_id_des<<endl;
                }
            }
        } else {
            uint64_t starttime,endtime;
            starttime = get_sys_clock();
            for (uint64_t i=0;i<msg->data_size;i++){
                //std::cout<<"i is "<<i<<endl;
                itemid_t* item = new(itemid_t);
                RC rc=WAIT;
                while (rc != RCOK){
                    rc = ((YCSBWorkload*)_wl)->the_index->index_read(key,item,key_to_part(key),g_thread_cnt); 
                }
                //std::cout<<"index_read!"<<endl;
                /*
                if (rc!=RCOK) {
                    miss_cnt++;
                    if (KEY_TO_PART == HASH_MODE) key += g_part_cnt;
                    else key ++;
                    continue;
                }
                */
                //std::cout<<"OK"<<endl;
                //((YCSBWorkload*)_wl)->the_index->index_read(key,item,msg->part_id,get_thd_id());        
                row_t* row = new(row_t);
                row = ((row_t*)item->location);
                //rc = txn_man->get_lock(row,WR); fix
                row_t* row_rtn = new(row_t);
                access_t access;
                #if (MIGRATION_ALG == REMUS )
                    access = access_t::WR;
                #elif (MIGRATION_ALG == DETEST)
                    access = access_t::RD;
                #elif (MIGRATION_ALG == DETEST_SPLIT)
                    access = access_t::RD;
                #else
                    access = access_t::WR;
                #endif
                #if CC_ALG == NO_WAIT || CC_ALG==WAIT_DIE || CC_ALG == DL_DELETE
                    access = access_t::WR;
                #endif
                rc = txn_man->get_row(row,access,row_rtn);
                if (rc != RCOK){
                    std::cout<<"trying to get lock..."<<endl;
                }
                while(rc != RCOK){
                    rc = txn_man->get_row(row,access,row_rtn);
                }

                msg->isdata=true;
                msg->data.emplace_back(*row_rtn);
                std::cout<<msg->data.back().get_primary_key()<<endl;
                //读row->data的数据
                char tmp_char[MAX_TUPLE_SIZE];
                strcpy(tmp_char, row_rtn->get_data());
                string tmp_str = tmp_char;
                
                //std::cout<<"tuple_size is "<<row->tuple_size<<' ';
                //std::cout<<"tmp_char is "<<tmp_char[0]<<' ';
                //std::cout<<"tmp_str is "<<tmp_str<<endl;
                msg->row_data.emplace_back(tmp_str);
                
                //std::cout<<"copy "<<key<<" ";
                //std::cout<<"key is "<<msg->data[i].get_primary_key();
                //std::cout<<"data is "<<msg->row_data[i]<<endl;
                //std::cout<<"lock "<<key<<' ';
                

                uint64_t x = PART_CNT;
                if (KEY_TO_PART == HASH_MODE) key = key + x;
                else key ++;
                //std::cout<<"key is "<<key<<endl;
            }
            endtime = get_sys_clock();
            std::cout<<"read data end! Time:"<<(endtime-starttime) / BILLION<<endl;
            
            //printf("the size of msg row is %ld\n",msg->data_size);
            //std::cout<<"the size of msg is "<<msg->get_size()<<endl;

            if(rc != WAIT) {
                MigrationMessage * msg1 = new(MigrationMessage);
                *msg1 = *msg;
                msg1->return_node_id = g_node_id;
                msg1->rtype = RECV_MIGRATION;
                msg_queue.enqueue(get_thd_id(), msg1, msg1->node_id_des);
                std::cout<<"enqueue finished!"<<endl;
            }
        }
        #if MIGRATION_ALG == DETEST
            update_minipart_map_status(msg->minipart_id, 1);
            update_part_map_status(msg->part_id, 1);
            if (detest_status == 0){
                update_detest_status(1);
            }
        #elif MIGRATION_ALG == REMUS
            update_part_map_status(msg->part_id, 1);
            if (remus_status == 0){
                update_remus_status(1);
            }
        #endif
    #endif
    std::cout<<"miss is:"<<miss_cnt<<endl;
    return rc;


#elif WORKLOAD == TPCC
    DEBUG("SEND_MIGRATION %ld\n",msg->get_txn_id());
    std::cout<<"SEND_MIGRATION Time:"<<(get_sys_clock() - g_starttime) / BILLION<<endl;
    if (g_mig_starttime == 0) g_mig_starttime = get_sys_clock();
    std::cout<<&msg<<endl;
    RC rc =RCOK;
    start_time = get_sys_clock();
    txn_man->return_id = msg->return_node_id;
    txn_man->h_wl = _wl;
    update_part_map_status(msg->part_id, 1); //migrating
    #if (MIGRATION_ALG == DETEST)
        update_detest_status(1);
        update_minipart_map_status(msg->minipart_id, 1);
    #elif (MIGRATION_ALG == DETEST_SPLIT)
        if (detest_status == 0){
            update_detest_status(1);
        }
        update_row_map_status_order(msg->order,1);
    #elif (MIGRATION_ALG == REMUS)
        update_remus_status(1);
        update_part_map_status(msg->part_id, 1);
    #elif (MIGRATION_ALG == SQUALL)
        update_squall_status(1);
        update_squallpart_map_status(msg->minipart_id, 1);
        update_part_map_status(msg->part_id, 1);
    #endif
    
    
    msg->isdata = false;

    #if MIGRATION_ALG == DETEST_SPLIT
        idx_key_t key;
        uint64_t label = Order[msg->order]; 
        for (uint64_t i=0;i<order_map[label].size();i++){ 
            key = order_map[label][i];
            itemid_t* item1 = (itemid_t*)mem_allocator.alloc(sizeof(itemid_t));
	        itemid_t* &item = item1;
            RC rc = ((YCSBWorkload*)_wl)->the_index->index_read(key,item,key_to_part(key),g_thread_cnt);
            row_t* row = ((row_t*)item->location);
            row_t* row_rtn = new(row_t);
            access_t access;
            access = access_t::RD;
            rc = txn_man->get_row(row,access,row_rtn);
            if (rc != RCOK){
                std::cout<<"trying to get lock..."<<endl;
            }
            while(rc != RCOK){
                rc = txn_man->get_row(row,access,row_rtn);
            }
        }
    #else
        
        uint64_t key = ((MigrationMessage*)msg)->key_start;
        std::cout<<"key is "<<key<<endl;
        std::cout<<msg->data_size<<endl;
        //std::cout<<"read data start!"<<endl;

        if (msg->data_size * g_tuplesize[2] > (MSG_CHUNK_SIZE - 200)){
            uint64_t tuple_num = MSG_CHUNK_SIZE / g_tuplesize[2] - 10; 
            uint64_t msg_num; 
            msg_num = msg->data_size / tuple_num + 1; 
            
            for (uint i=0;i<msg_num;i++){
                MigrationMessage* msg1 = new(MigrationMessage);
                *msg1 = *msg;
                if (i != msg_num-1) {
                    msg1->data_size = tuple_num;
                    msg1->islast = false;
                }
                else {
                    msg1->data_size = msg->data_size - (msg_num -1)*tuple_num;
                    msg1->islast = true;
                }
                msg1->key_start = i * tuple_num + msg->key_start;

                //int64_t starttime,endtime;
                //starttime = get_sys_clock();
                for (uint64_t j=0;j<msg1->data_size;j++){
                    itemid_t* item = new(itemid_t);
                    RC rc=WAIT;
                    while (rc != RCOK){
                        INDEX * index = ((TPCCWorkload*)_wl)->i_customer_id;
                        rc = index->index_read(key,item,MIGRATION_PART,g_thread_cnt);
                    }
                    //std::cout<<"index_read!"<<endl;

                    row_t* row = new(row_t);
                    row = ((row_t*)item->location);
                    row_t* row_rtn = new(row_t);
                    access_t access;
                    #if (MIGRATION_ALG == REMUS )
                        access = access_t::WR;
                    #elif (MIGRATION_ALG == DETEST)
                        access = access_t::WR;
                    #elif (MIGRATION_ALG == DETEST_SPLIT)
                        access = access_t::RD;
                    #else
                        access = access_t::WR;
                    #endif
                    #if CC_ALG == NO_WAIT || CC_ALG==WAIT_DIE || CC_ALG == DL_DELETE
                    access = access_t::WR;
                    #endif
                    rc = txn_man->get_row(row,access,row_rtn);
                    if (rc != RCOK){
                        std::cout<<"trying to get lock..."<<endl;
                    }
                    while(rc != RCOK){
                        rc = txn_man->get_row(row,access,row_rtn);
                    }
                    //std::cout<<"get_row!"<<endl;
                
                    msg1->isdata=true;
                    msg1->data.emplace_back(*row_rtn);
                    
                    //std::cout<<"key is "<<msg1->data[i].get_primary_key();
                    //std::cout<<"data is "<<msg1->row_data[i]<<endl;

                    key ++;
                }
                //endtime = get_sys_clock();
                //std::cout<<"read data end! Time:"<<(endtime-starttime) / BILLION<<endl;
                //printf("the size of msg row is %ld\n",msg1->data_size);
                std::cout<<"the size of msg is "<<msg1->get_size()<<endl;

                if(rc != WAIT) {
                    msg1->return_node_id = g_node_id;
                    msg1->rtype = RECV_MIGRATION;
                    msg_queue.enqueue(get_thd_id(), msg1, msg1->node_id_des);
                    std::cout<<"enqueue finished "<<i<<endl;
                }
            }
        } else {
            uint64_t starttime,endtime;
            starttime = get_sys_clock();
            for (uint64_t i=0;i<msg->data_size;i++){
                //std::cout<<"i is "<<i<<endl;
                itemid_t* item = new(itemid_t);
                RC rc=WAIT;
                while (rc != RCOK){
                    INDEX * index = ((TPCCWorkload*)_wl)->i_customer_id;
                    rc = index->index_read(key,item,MIGRATION_PART,g_thread_cnt);
                }
                //std::cout<<"index_read!"<<endl;
                /*
                if (rc!=RCOK) {
                    miss_cnt++;
                    if (KEY_TO_PART == HASH_MODE) key += g_part_cnt;
                    else key ++;
                    continue;
                }
                */
                //std::cout<<"OK"<<endl;
                //((YCSBWorkload*)_wl)->the_index->index_read(key,item,msg->part_id,get_thd_id());        
                row_t* row = new(row_t);
                row = ((row_t*)item->location);
                //rc = txn_man->get_lock(row,WR); fix
                row_t* row_rtn = new(row_t);
                access_t access;
                #if (MIGRATION_ALG == REMUS )
                    access = access_t::WR;
                #elif (MIGRATION_ALG == DETEST)
                    access = access_t::RD;
                #elif (MIGRATION_ALG == DETEST_SPLIT)
                    access = access_t::RD;
                #else
                    access = access_t::WR;
                #endif
                #if CC_ALG == NO_WAIT || CC_ALG==WAIT_DIE || CC_ALG == DL_DELETE
                    access = access_t::WR;
                #endif
                rc = txn_man->get_row(row,access,row_rtn);
                if (rc != RCOK){
                    std::cout<<"trying to get lock..."<<endl;
                }
                while(rc != RCOK){
                    rc = txn_man->get_row(row,access,row_rtn);
                }
                //std::cout<<"get_row!"<<endl;

                msg->isdata=true;
                msg->data.emplace_back(*row_rtn);
                std::cout<<msg->data.back().get_primary_key()<<endl;
                
                key ++;
                //std::cout<<"key is "<<key<<endl;
            }
            endtime = get_sys_clock();
            std::cout<<"read data end! Time:"<<(endtime-starttime) / BILLION<<endl;
            
            //printf("the size of msg row is %ld\n",msg->data_size);
            //std::cout<<"the size of msg is "<<msg->get_size()<<endl;


            if(rc != WAIT) {
                MigrationMessage * msg1 = new(MigrationMessage);
                *msg1 = *msg;
                msg1->return_node_id = g_node_id;
                msg1->rtype = RECV_MIGRATION;
                msg_queue.enqueue(get_thd_id(), msg1, msg1->node_id_des);
                std::cout<<"enqueue finished!"<<endl;
            }
        }
        #if MIGRATION_ALG == DETEST
            update_minipart_map_status(msg->minipart_id, 1);
            update_part_map_status(msg->part_id, 1);
            if (detest_status == 0){
                update_detest_status(1);
            }
        #elif MIGRATION_ALG == REMUS
            update_part_map_status(msg->part_id, 1);
            if (remus_status == 0){
                update_remus_status(1);
            }
        #endif
    #endif
    std::cout<<"miss is:"<<miss_cnt<<endl;
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
    return rc;
#endif
}

RC MigrateThread::process_recv_migration(MigrationMessage* msg){
    DEBUG("RECV_MIGRATION %ld\n",msg->get_txn_id());
<<<<<<< HEAD
    std::cout<<"RECV_MIGRATION Time:"<<(get_server_clock() - g_start_time) / BILLION<<endl;
    RC rc = RCOK;

    //debug
    //for (uint i=0; i< msg->mig_order.size(); i++) std::cout<<msg->mig_order[i]<<' ';        

    //receive data and construct
    uint64_t key_ptr;//received key
    key_ptr = ((MigrationMessage*)msg)->key_start;
    while (key_ptr <= ((MigrationMessage*)msg)->key_end){
        row_t * new_row = NULL;
        uint64_t row_id;
        rc = this->_wl->tables["MAIN_TABLE"]->get_new_row(new_row, ((MigrationMessage*)msg)-> part_id, row_id);
=======
    std::cout<<"RECV_MIGRATION Time:"<<(get_server_clock() - g_starttime) / BILLION<<endl;
    RC rc = RCOK;
    if (g_mig_starttime == 0) g_mig_starttime = get_sys_clock();
    //std::cout<<"msg data_size is: "<<msg->data_size<<endl;
    //std::cout<<"minipart_id: "<<((MigrationMessage*)msg)->minipart_id<<' ';
    
    /*fix
    row_t * data_ptr = (row_t*)mem_allocator.alloc(sizeof(row_t)*(msg->data_size));
    //char* row_data_ptr = (char* )malloc((msg->data[0].tuple_size) *  (msg->data_size));
    for (size_t i=0;i<msg->data_size;i++){
        row_t* data_ = &msg->data[i];
        memcpy(data_ptr,data_,sizeof(row_t));
        //memcpy(row_data_ptr,&msg->row_data,msg->data[0].tuple_size);
    }
    
    void* data_ptr = mem_allocator.alloc(sizeof(row_t)*(msg->data_size)); //存row_t
    char* row_data_ptr = (char* )malloc((msg->data[0].tuple_size) *  (msg->data_size)); //存row_t->data
    uint64_t ptr = 0;
    uint64_t ptr_data = 0;
    for (size_t i=0;i<msg->data_size;i++){
        row_t* data_ = &msg->data[i];
        memcpy(data_ptr,(const void*)data_,ptr);
        ptr += sizeof(row_t);
        memcpy(row_data_ptr,(const void*)&msg->row_data,ptr_data);
        ptr_data += sizeof(data_->get_tuple_size());
        //本地生成索引 索引是全局的不需要再次生成
        
        ((YCSBWorkload*)_wl)->the_table->get_new_row(data_);
        itemid_t * m_item = (itemid_t *) mem_allocator.alloc(sizeof(itemid_t));
	    assert(m_item != NULL);
	    m_item->type = DT_row;
	    m_item->location = row_data_ptr+ptr_data-sizeof(data_->get_tuple_size());
	    m_item->valid = true;
	    uint64_t idx_key = data_->get_primary_key();
	    rc = ((YCSBWorkload*)_wl)->the_index->index_insert(idx_key, m_item, msg->part_id);
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
        assert(rc == RCOK);
        uint64_t primary_key = key_ptr;
        new_row->set_primary_key(primary_key);

        #if SIM_FULL_ROW
            new_row->set_value(0, &primary_key,sizeof(uint64_t));
            Catalog * schema = this->_wl->tables["MAIN_TABLE"]->get_schema();
            for (UInt32 fid = 0; fid < schema->get_field_cnt(); fid ++) {
                char value[6] = "hello";
                new_row->set_value(fid, value,sizeof(value));				
            }
        #endif        

<<<<<<< HEAD
        itemid_t * m_item = (itemid_t *) mem_allocator.alloc( sizeof(itemid_t));
        assert(m_item != NULL);
        m_item->type = DT_row;
        m_item->location = new_row;
        m_item->valid = true;
        uint64_t idx_key = primary_key;
        rc = this->_wl->indexes["MAIN_INDEX"]->index_insert(idx_key, m_item, ((MigrationMessage*)msg)->part_id);
        assert(rc == RCOK);

        key_ptr += PART_CNT;
=======
    if (msg->islast){ 
        update_part_map_status(msg->part_id, 1); //migrating
        #if (MIGRATION_ALG == DETEST)
            if (detest_status == 0){
                update_detest_status(1);
            }
            update_minipart_map_status(msg->minipart_id, 1);
            update_part_map_status(msg->part_id, 1);
            #if REMUS_SPLIT
                if (remus_status == 0){
                    update_remus_status(1);
                }
                else {
                    std::cout<<"%%%%%%%%%%remus != 0"<<endl;
                }
                update_part_map_status(msg->part_id, 1);
            #endif
        #elif (MIGRATION_ALG == DETEST_SPLIT)
            if (detest_status == 0){
                update_detest_status(1);
            }
            update_row_map_status_order(msg->order,1);
        #elif (MIGRATION_ALG == REMUS)
            if (remus_status == 0){
                update_remus_status(1);
            }
            update_part_map_status(msg->part_id, 1);
        #elif (MIGRATION_ALG == SQUALL)
            if (remus_status == 0){
                update_squall_status(1);
            }
            update_squallpart_map_status(msg->minipart_id, 1);
            update_part_map_status(msg->part_id, 1);
        #endif

        #if WORKLOAD == YCSB
            #if MIGRATION_ALG == DETEST_SPLIT
                uint64_t key;
                uint64_t label = Order[msg->order]; 
                for (uint64_t i=0;i<order_map[label].size();i++){ 
                    key = order_map[label][i];
                    row_t * new_row = NULL;
                    uint64_t row_id;
                    rc = this->_wl->tables["MAIN_TABLE"]->get_new_row(new_row, msg -> part_id, row_id);
                    assert(rc == RCOK);
                    uint64_t primary_key = key;
                    new_row->set_primary_key(primary_key);
                    #if SIM_FULL_ROW
                        new_row->set_value(0, &primary_key,sizeof(uint64_t));
                        Catalog * schema = this->_wl->tables["MAIN_TABLE"]->get_schema();
                        for (UInt32 fid = 0; fid < schema->get_field_cnt(); fid ++) {
                            char value[6] = "hello";
                            new_row->set_value(fid, value,sizeof(value));				
                        }
                    #endif
                    itemid_t * m_item = (itemid_t *) mem_allocator.alloc( sizeof(itemid_t));
                    assert(m_item != NULL);
                    m_item->type = DT_row;
                    m_item->location = new_row;
                    m_item->valid = true;
                    uint64_t idx_key = primary_key;
                    rc = this->_wl->indexes["MAIN_INDEX"]->index_insert(idx_key, m_item, msg->part_id);
                    //std::cout<<key<<' ';
                    assert(rc == RCOK);
                }
            #elif MIGRATION_ALG == REMUS
                uint64_t key,slice_size;
                key = msg->part_id;
                slice_size = g_synth_table_size / g_part_cnt; 
                for (uint64_t i = 0; i < slice_size; i++){
                    row_t * new_row = NULL;
                    uint64_t row_id;
                    rc = this->_wl->tables["MAIN_TABLE"]->get_new_row(new_row, msg -> part_id, row_id);
                    assert(rc == RCOK);
                    uint64_t primary_key = key;
                    new_row->set_primary_key(primary_key);
                    #if SIM_FULL_ROW
                        new_row->set_value(0, &primary_key,sizeof(uint64_t));
                        Catalog * schema = this->_wl->tables["MAIN_TABLE"]->get_schema();
                        for (UInt32 fid = 0; fid < schema->get_field_cnt(); fid ++) {
                            char value[6] = "hello";
                            new_row->set_value(fid, value,sizeof(value));				
                        }
                    #endif
                    itemid_t * m_item = (itemid_t *) mem_allocator.alloc( sizeof(itemid_t));
                    assert(m_item != NULL);
                    m_item->type = DT_row;
                    m_item->location = new_row;
                    m_item->valid = true;
                    uint64_t idx_key = primary_key;
                    rc = this->_wl->indexes["MAIN_INDEX"]->index_insert(idx_key, m_item, msg->part_id);
                    assert(rc == RCOK);
                    key += g_part_cnt;
                }
            #elif MIGRATION_ALG == DETEST
                uint64_t key,slice_size;
                key = msg->part_id + msg->minipart_id * (g_synth_table_size / g_part_cnt / PART_SPLIT_CNT) * g_part_cnt;
                slice_size = g_synth_table_size / g_part_cnt / PART_SPLIT_CNT;
                for (uint64_t i = 0; i < slice_size; i++){
                    row_t * new_row = NULL;
                    uint64_t row_id;
                    rc = this->_wl->tables["MAIN_TABLE"]->get_new_row(new_row, msg -> part_id, row_id);
                    assert(rc == RCOK);
                    uint64_t primary_key = key;
                    new_row->set_primary_key(primary_key);
                    #if SIM_FULL_ROW
                        new_row->set_value(0, &primary_key,sizeof(uint64_t));
                        Catalog * schema = this->_wl->tables["MAIN_TABLE"]->get_schema();
                        for (UInt32 fid = 0; fid < schema->get_field_cnt(); fid ++) {
                            char value[6] = "hello";
                            new_row->set_value(fid, value,sizeof(value));				
                        }
                    #endif
                    itemid_t * m_item = (itemid_t *) mem_allocator.alloc( sizeof(itemid_t));
                    assert(m_item != NULL);
                    m_item->type = DT_row;
                    m_item->location = new_row;
                    m_item->valid = true;
                    uint64_t idx_key = primary_key;
                    rc = this->_wl->indexes["MAIN_INDEX"]->index_insert(idx_key, m_item, msg->part_id);
                    assert(rc == RCOK);
                    key += g_part_cnt;
                }
            #elif MIGRATION_ALG == SQUALL
                uint64_t key,slice_size;
                key = msg->part_id + msg->minipart_id * (g_synth_table_size / g_part_cnt / Squall_Part_Cnt) * g_part_cnt;
                slice_size = g_synth_table_size / g_part_cnt / Squall_Part_Cnt;
                for (uint64_t i = 0; i < slice_size; i++){
                    row_t * new_row = NULL;
                    uint64_t row_id;
                    rc = this->_wl->tables["MAIN_TABLE"]->get_new_row(new_row, msg -> part_id, row_id);
                    assert(rc == RCOK);
                    uint64_t primary_key = key;
                    new_row->set_primary_key(primary_key);
                    #if SIM_FULL_ROW
                        new_row->set_value(0, &primary_key,sizeof(uint64_t));
                        Catalog * schema = this->_wl->tables["MAIN_TABLE"]->get_schema();
                        for (UInt32 fid = 0; fid < schema->get_field_cnt(); fid ++) {
                            char value[6] = "hello";
                            new_row->set_value(fid, value,sizeof(value));				
                        }
                    #endif
                    itemid_t * m_item = (itemid_t *) mem_allocator.alloc( sizeof(itemid_t));
                    assert(m_item != NULL);
                    m_item->type = DT_row;
                    m_item->location = new_row;
                    m_item->valid = true;
                    uint64_t idx_key = primary_key;
                    rc = this->_wl->indexes["MAIN_INDEX"]->index_insert(idx_key, m_item, msg->part_id);
                    assert(rc == RCOK);
                    key += g_part_cnt;
                }        
            #endif
        #elif WORKLOAD == TPCC
            #if MIGRATION_ALG == DETEST_SPLIT
            #else
                uint64_t wid = 1;
		        for (uint64_t did = 1; did <= g_dist_per_wh; did++) {
                    for (UInt32 cid = 1; cid <= g_cust_per_dist; cid += 1){
                        row_t * row;
                        uint64_t row_id;
                        ((TPCCWorkload*)this->_wl)->t_customer->get_new_row(row, 0, row_id);
                        row->set_primary_key(cid);

                        row->set_value(C_ID, cid);
                        row->set_value(C_D_ID, did);
                        row->set_value(C_W_ID, wid);
                        char c_last[LASTNAME_LEN];
                        if (cid <= 1000)
                            Lastname(cid - 1, c_last);
                        else
                            Lastname(NURand(255,0,999), c_last);
                        row->set_value(C_LAST, c_last);
                        if (RAND(10) == 0) {
                            char tmp[] = "GC";
                            row->set_value(C_CREDIT, tmp);
                        } else {
                            char tmp[] = "BC";
                            row->set_value(C_CREDIT, tmp);
                        }
                        row->set_value(C_DISCOUNT, (double)RAND(5000) / 10000);
                        row->set_value(C_BALANCE, -10.0);
                        row->set_value(C_YTD_PAYMENT, 10.0);
                        row->set_value(C_PAYMENT_CNT, 1);
                        uint64_t key;
                        key = custNPKey(c_last, did, wid);
                        ((TPCCWorkload*)this->_wl)->index_insert(((TPCCWorkload*)this->_wl)->i_customer_last, key, row, wh_to_part(wid));
                        key = custKey(cid, did, wid);
                        ((TPCCWorkload*)this->_wl)->index_insert(((TPCCWorkload*)this->_wl)->i_customer_id, key, row, wh_to_part(wid));
                    }
                }
                
		        for (uint64_t did = 1; did <= g_dist_per_wh; did++)
	                for (uint64_t cid = 1; cid <= g_cust_per_dist; cid++) 
                        ((TPCCWorkload*)this->_wl)->init_tab_hist(cid, did, wid);	
            #endif
        #endif

        if (rc==RCOK){
            MigrationMessage * msg1 = new(MigrationMessage);
            *msg1 = *msg;
            msg1->rtype = FINISH_MIGRATION;
            #if MIGRATION_ALG == DETEST
                msg1->minipart_id = msg->minipart_id;
            #elif MIGRATION_ALG == DETEST_SPLIT
                msg1->order = msg->order;
            #endif
            msg_queue.enqueue(get_thd_id(), msg1, msg1->node_id_src);
            std::cout<<"enqueue finished!"<<endl<<"part is "<<msg1->part_id<<endl<<"node_id_src is "<<msg1->node_id_src<<endl<<"node_id_des is "<<msg1->node_id_des<<endl;
            std::cout<<(get_server_clock() - g_starttime) / BILLION<<endl;
            g_mig_endtime = get_sys_clock();

        }
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
    }

    //update migration metadata 
    if (((MigrationMessage*)msg)->islast){
        update_minipart_map_status(
            ((MigrationMessage*)msg)->part_id,
            ((MigrationMessage*)msg)->minipart_id, 1);
        update_part_map_status(((MigrationMessage*)msg)->part_id, 1);

        //send finish migration msg
        MigrationMessage * msg1 = new(MigrationMessage);
        *msg1 = *msg;
        msg1->rtype = FINISH_MIGRATION;
        msg1->isdata = false;
        #if MIGRATION_ALG == DETEST
            msg1->minipart_id = msg->minipart_id;
        #endif
        msg_queue.enqueue(get_thd_id(), msg1, msg1->node_id_src);
        std::cout<<"Recv migration from "<<msg1->node_id_src<<" to "<<msg1->node_id_des<<" : enqueue finished "<<endl<<"part is "<<msg1->part_id<<endl;
        std::cout<<(get_server_clock() - g_start_time) / BILLION<<endl;
        g_mig_endtime = get_sys_clock();
    }    

    txn_man->commit();
    return rc;
}

RC MigrateThread::process_finish_migration(MigrationMessage* msg){
    DEBUG("FINISH_MIGRATION %ld\n",msg->get_txn_id());
<<<<<<< HEAD
    std::cout<<"FINISH_MIGRATION Time:"<<(get_server_clock() - g_start_time) / BILLION<<endl;
    RC rc = RCOK;

    assert(msg->islast);

    //debug
    //for (uint i=0; i< msg->mig_order.size(); i++) std::cout<<msg->mig_order[i]<<' ';    

    //update migration metadata
    #if MIGRATION_ALG == DETEST
        // update_minipart_map(
        //    msg->part_id, msg->minipart_id, msg->node_id_des);
        // update_minipart_map_status(msg->part_id, msg->minipart_id, 2);
        std::cout<<"Finish migration from "<<msg->node_id_src<<" to "<<msg->node_id_des<<" part_id "<<msg->part_id<<" minipart_id "<<msg->minipart_id<<endl;

        //sync msg to other nodes    
        for (uint64_t i=0; i< g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_minipartmap_message(SET_MINIPARTMAP, msg->part_id, msg->minipart_id, msg->node_id_des, 2), i);        
        } 
    
    
        //construct new migration msg
        if (msg->order == g_part_split_cnt -1){//finish migrating all minipart in this part
            //update_part_map(msg->part_id, msg->node_id_des);
            //update_part_map_status(msg->part_id, 2);

            for (uint64_t i=0; i< g_node_cnt + g_client_node_cnt; i++){
                if (i == g_node_id) continue;
                msg_queue.enqueue(get_thd_id(),Message::create_partmap_message(SET_PARTMAP, msg->part_id,  msg->node_id_des, 2), i);        
            }               
        } else{ //send next minipart migration msg
			uint64_t node_id_src_ = msg->node_id_src;
			uint64_t node_id_des_ = msg->node_id_des;
			uint64_t part_id_ = msg->part_id;
			uint64_t minipart_id_ = msg->mig_order[msg->order + 1];
            uint64_t order_ = msg->order + 1;
			bool islast_ = true;

            uint64_t data_size_ = g_synth_table_size / g_part_cnt / g_part_split_cnt;
            uint64_t key_start_ = minipart_to_key_start(part_id_, minipart_id_);
            uint64_t key_end_ = minipart_to_key_end(part_id_, minipart_id_);
            islast_ = false;   

            //debug
            //for (uint i=0; i< msg->mig_order.size(); i++) std::cout<<msg->mig_order[i]<<' ';
            //std::cout<<endl<<"part_id "<<part_id_<<" minpart_id "<<minipart_id_<<endl;


            MigrationMessage * msg1 = new(MigrationMessage);
            *msg1 = *msg;
            ((MigrationMessage*)msg1)->node_id_src = node_id_src_;
            ((MigrationMessage*)msg1)->node_id_des = node_id_des_;
            ((MigrationMessage*)msg1)->part_id = part_id_;
            ((MigrationMessage*)msg1)->minipart_id = minipart_id_;
            ((MigrationMessage*)msg1)->rtype = SEND_MIGRATION;
            ((MigrationMessage*)msg1)->data_size = data_size_;
            ((MigrationMessage*)msg1)->return_node_id = node_id_des_;
            ((MigrationMessage*)msg1)->isdata = islast_;
            ((MigrationMessage*)msg1)->key_start = key_start_;			
            ((MigrationMessage*)msg1)->key_end = key_end_;	
            ((MigrationMessage*)msg1)->order = order_;		
            ((MigrationMessage*)msg1)->txn_id = msg->get_txn_id();      

            //debug
            //std::cout<<"minipart_id: "<<((MigrationMessage*)msg1)->minipart_id<<' ';

=======
    std::cout<<"FINISH_MIGRATION Time:"<<(get_server_clock() - g_starttime) / BILLION<<endl;
    RC rc = RCOK;
    //msg->copy_to_txn(txn_man);
    //txn_man->h_wl = _wl;

#if WORKLOAD == YCSB    
    #if (MIGRATION_ALG == DETEST)
        update_minipart_map(msg->minipart_id, msg->node_id_des);
        update_minipart_map_status(msg->minipart_id, 2);
        std::cout<<"minipart "<<msg->minipart_id<<" status is 2"<<endl;
        //sync message to other nodes
        for (uint64_t i=0; i< g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_MINIPARTMAP, msg->minipart_id, msg->node_id_des, 2), i);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->minipart_id, 2),i);            
        }
       
        #if REMUS_SPLIT
             remus_finish_time = get_sys_clock();
            //update_part_map(msg->part_id, msg->node_id_des);
            update_part_map_status(msg->part_id, 2);//copied
            update_remus_status(2);
            //sync msg to other nodes
            for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
                if (i == g_node_id) continue;
                msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_src, 2), i);//stage2还没切主
                msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 2),i);            
            }
            std::cout<<"remus status is "<<remus_status<<"Time is: "<<(get_sys_clock() - g_starttime) / BILLION <<endl;

            sleep(SYNCTIME); //for remus stage2

            update_remus_status(3);
            update_part_map_status(msg->part_id, 3);
            //sync msg to other nodes
            for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
                if (i == g_node_id) continue;
                msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 3), i);
                msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 3),i);            
            }
            std::cout<<"remus status is "<<remus_status<<"Time is: "<<(get_sys_clock() - g_starttime) / BILLION <<endl;
            std::cout<<"part is "<<msg->part_id<<endl<<"node_id_src is "<<msg->node_id_src<<endl<<"node_id_des is "<<msg->node_id_des<<endl;
        #endif

        #if (COSTENABLE == true)
            std::cout<<"**********"<<endl;
            if (msg->minipart_id == PART_SPLIT_CNT-1){
                update_detest_status(2);
                update_part_map_status(msg->part_id, 2);//migrated
                //sync message to other nodes
                for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
                    if (i == g_node_id) continue;
                    msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2), i);
                    msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->part_id, 2),i);
                }            
            } 
            else if (msg->minipart_id < PART_SPLIT_CNT-1) {
                Message * msg1 = Message::create_message(SEND_MIGRATION);
                ((MigrationMessage*)msg1)->node_id_src = msg->node_id_src;
                ((MigrationMessage*)msg1)->node_id_des = msg->node_id_des;
                ((MigrationMessage*)msg1)->part_id = msg->part_id;
                ((MigrationMessage*)msg1)->minipart_id = msg->minipart_id + 1;
                ((MigrationMessage*)msg1)->rtype = SEND_MIGRATION;
                ((MigrationMessage*)msg1)->data_size = g_synth_table_size / g_part_cnt / PART_SPLIT_CNT;
                ((MigrationMessage*)msg1)->return_node_id = msg->node_id_des;
                ((MigrationMessage*)msg1)->isdata = false;
                ((MigrationMessage*)msg1)->key_start = ((MigrationMessage*)msg1)->part_id + ((MigrationMessage*)msg1)->minipart_id * (g_synth_table_size / PART_SPLIT_CNT / g_part_cnt) * g_part_cnt;
                ((MigrationMessage*)msg1)->txn_id = msg->get_txn_id();        
                std::cout<<"minipart_id: "<<((MigrationMessage*)msg1)->minipart_id<<' ';
                start_time = get_sys_clock();
                txn_man->return_id = msg1->return_node_id;
                txn_man->h_wl = _wl;
                std::cout<<"the size of msg is "<<msg1->get_size()<<endl;

                #if REMUS_SPLIT 
                    update_remus_status(0);
                    for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
                        if (i == g_node_id) continue;
                        msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 0),i);
                    }  
                #endif

                process_send_migration((MigrationMessage*)msg1);
            }        

        #elif (COSTENABLE == false)
            std::cout<<"msg->minipart is "<<msg->minipart_id<<endl;
            if (msg->minipart_id == 0){
                std::cout<<"!!!!!!!!!!!!!!"<<endl;
                //update_part_map(msg->part_id, msg->node_id_des);
                update_detest_status(2);
                update_part_map_status(msg->part_id, 2);//migrated
                //sync message to other nodes
                for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
                    if (i == g_node_id) continue;
                    msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2), i);
                    msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->part_id, 2),i);

                }            
            }
            else if (msg->minipart_id > 0) {
                std::cout<<"*********"<<endl;
                Message * msg1 = Message::create_message(SEND_MIGRATION);
                ((MigrationMessage*)msg1)->node_id_src = msg->node_id_src;
                ((MigrationMessage*)msg1)->node_id_des = msg->node_id_des;
                ((MigrationMessage*)msg1)->part_id = msg->part_id;
                ((MigrationMessage*)msg1)->minipart_id = msg->minipart_id - 1;
                ((MigrationMessage*)msg1)->rtype = SEND_MIGRATION;
                ((MigrationMessage*)msg1)->data_size = g_synth_table_size / g_part_cnt / PART_SPLIT_CNT;
                ((MigrationMessage*)msg1)->return_node_id = msg->node_id_des;
                ((MigrationMessage*)msg1)->isdata = false;
                ((MigrationMessage*)msg1)->key_start = ((MigrationMessage*)msg1)->part_id + ((MigrationMessage*)msg1)->minipart_id * (g_synth_table_size / PART_SPLIT_CNT / g_part_cnt) * g_part_cnt;
                ((MigrationMessage*)msg1)->txn_id = msg->get_txn_id();
                
                std::cout<<"minipart_id: "<<((MigrationMessage*)msg1)->minipart_id<<' ';
                start_time = get_sys_clock();
                txn_man->return_id = msg1->return_node_id;
                txn_man->h_wl = _wl;
                std::cout<<"the size of msg is "<<msg1->get_size()<<endl;

                process_send_migration((MigrationMessage*)msg1);
            }            
        #endif
    #elif (MIGRATION_ALG == SQUALL)
        update_squallpart_map(msg->minipart_id, msg->node_id_des);
        update_squallpart_map_status(msg->minipart_id, 2);
        std::cout<<"squallpart "<<msg->minipart_id<<" status is 2"<<endl;
        //sync msg to other nodes
        for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_SQUALLPARTMAP, msg->minipart_id, msg->node_id_des, 2), i);
        }
    #elif (MIGRATION_ALG == REMUS)
        //sleep(SYNCTIME);  
        remus_finish_time = get_sys_clock();
        //update_part_map(msg->part_id, msg->node_id_des);
        update_part_map_status(msg->part_id, 2);//copied
        update_remus_status(2);
        //sync msg to other nodes
        for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_src, 2), i);//stage2还没切主
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 2),i);            
        }
        std::cout<<"remus status is "<<remus_status<<"Time is: "<<(get_sys_clock() - g_starttime) / BILLION <<endl;

        sleep(SYNCTIME); //for remus stage2

        update_remus_status(3);
        update_part_map_status(msg->part_id, 3);
        //sync msg to other nodes
        for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 3), i);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 3),i);            
        }
        std::cout<<"remus status is "<<remus_status<<"Time is: "<<(get_sys_clock() - g_starttime) / BILLION <<endl;
        std::cout<<"part is "<<msg->part_id<<endl<<"node_id_src is "<<msg->node_id_src<<endl<<"node_id_des is "<<msg->node_id_des<<endl;
    #elif (MIGRATION_ALG == DETEST_SPLIT)
        update_row_map_order(msg->order, msg->node_id_des);
        update_row_map_status_order(msg->order, 2);
        update_migrate_label(msg->order+1);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_ROWMAP, msg->order, msg->node_id_des, 2),g_node_cnt);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_ROWMAP, msg->order, msg->node_id_des, 2),msg->node_id_des);
        if (msg->order == DETEST_SPLIT-1){
            //update_part_map(msg->part_id, msg->node_id_des);
            update_part_map_status(msg->part_id, 2);//migrated
            update_migrate_label(msg->order+1);
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),g_node_cnt); //g_node_cnt对应着client节点,发消息通知client修改detest状态
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),msg->node_id_des);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->part_id, 2),msg->node_id_des);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->part_id, 2),g_node_cnt);
        }
    #else
        //update_part_map(msg->part_id, msg->node_id_des);
        update_part_map_status(msg->part_id, 2);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),g_node_cnt);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),msg->node_id_des);
    #endif
#elif WORKLOAD == TPCC
    #if (MIGRATION_ALG == DETEST)
        update_minipart_map(msg->minipart_id, msg->node_id_des);
        update_minipart_map_status(msg->minipart_id, 2);
        std::cout<<"minipart "<<msg->minipart_id<<" status is 2"<<endl;
        //sync message to other nodes
        for (uint64_t i=0; i< g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_MINIPARTMAP, msg->minipart_id, msg->node_id_des, 2), i);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->minipart_id, 2),i);            
        }
        if (msg->minipart_id == PART_SPLIT_CNT-1){
            //update_part_map(msg->part_id, msg->node_id_des);
            update_detest_status(2);
            update_part_map_status(msg->part_id, 2);//migrated
            //sync message to other nodes
            for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
                if (i == g_node_id) continue;
                msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2), i);
                msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->part_id, 2),i);
            }    
        } else if (((SetMiniPartMapMessage*)msg)->minipart_id < PART_SPLIT_CNT-1) {
            Message * msg1 = Message::create_message(SEND_MIGRATION);
			((MigrationMessage*)msg1)->node_id_src = msg->node_id_src;
			((MigrationMessage*)msg1)->node_id_des = msg->node_id_des;
			((MigrationMessage*)msg1)->part_id = msg->part_id;
			((MigrationMessage*)msg1)->minipart_id = msg->minipart_id + 1;
			((MigrationMessage*)msg1)->rtype = SEND_MIGRATION;
			((MigrationMessage*)msg1)->data_size = g_dist_per_wh * g_cust_per_dist / PART_SPLIT_CNT;
			((MigrationMessage*)msg1)->return_node_id = msg->node_id_des;
			((MigrationMessage*)msg1)->isdata = false;
			((MigrationMessage*)msg1)->key_start = ((MigrationMessage*)msg1)->minipart_id * g_dist_per_wh * g_cust_per_dist / PART_SPLIT_CNT + custKey(MIGRATION_PART+1, MIGRATION_PART+1, MIGRATION_PART+1);
            ((MigrationMessage*)msg1)->txn_id = msg->get_txn_id();

            
            std::cout<<"minipart_id: "<<((MigrationMessage*)msg1)->minipart_id<<' ';
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
            start_time = get_sys_clock();
            txn_man->return_id = msg1->return_node_id;
            txn_man->h_wl = _wl;
            std::cout<<"the size of msg is "<<msg1->get_size()<<endl;

            process_send_migration((MigrationMessage*)msg1);
        }
<<<<<<< HEAD
    #endif

    g_mig_time.emplace_back((get_sys_clock() - g_mig_tmp_time) / MILLION);
    std::cout<<"size of g_mig_time is "<<g_mig_time.size()<<endl;

=======
    #elif (MIGRATION_ALG == SQUALL)
        update_squallpart_map(msg->minipart_id, msg->node_id_des);
        update_squallpart_map_status(msg->minipart_id, 2);
        std::cout<<"squallpart "<<msg->minipart_id<<" status is 2"<<endl;
        //sync msg to other nodes
        for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_SQUALLPARTMAP, msg->minipart_id, msg->node_id_des, 2), i);
        }
    #elif (MIGRATION_ALG == REMUS)
        //sleep(SYNCTIME);  
        remus_finish_time = get_sys_clock();
        //update_part_map(msg->part_id, msg->node_id_des);
        update_part_map_status(msg->part_id, 2);//copied
        update_remus_status(2);
        //sync msg to other nodes
        for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_src, 2), i);//stage2还没切主
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 2),i);            
        }       
        /* 
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),g_node_cnt); 
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),msg->node_id_des);
        msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 2),g_node_cnt);
        msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 2),msg->node_id_des);
        */
        std::cout<<"remus status is "<<remus_status<<"Time is: "<<(get_sys_clock() - g_starttime) / BILLION <<endl;

        sleep(SYNCTIME); //for remus stage2

        update_remus_status(3);
        update_part_map_status(msg->part_id, 3);
        //sync msg to other nodes
        for (uint64_t i=0; i<g_node_cnt + g_client_node_cnt; i++){
            if (i == g_node_id) continue;
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 3), i);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 3),i);            
        }
        /*
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 3),g_node_cnt); 
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 3),msg->node_id_des);
        msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 3),g_node_cnt);
        msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_REMUS, msg->part_id, 3),msg->node_id_des);
        */
        std::cout<<"remus status is "<<remus_status<<"Time is: "<<(get_sys_clock() - g_starttime) / BILLION <<endl;
        std::cout<<"part is "<<msg->part_id<<endl<<"node_id_src is "<<msg->node_id_src<<endl<<"node_id_des is "<<msg->node_id_des<<endl;
    #elif (MIGRATION_ALG == DETEST_SPLIT)
        update_row_map_order(msg->order, msg->node_id_des);
        update_row_map_status_order(msg->order, 2);
        update_migrate_label(msg->order+1);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_ROWMAP, msg->order, msg->node_id_des, 2),g_node_cnt);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_ROWMAP, msg->order, msg->node_id_des, 2),msg->node_id_des);
        if (msg->order == DETEST_SPLIT-1){//
            //update_part_map(msg->part_id, msg->node_id_des);
            update_part_map_status(msg->part_id, 2);//migrated
            update_migrate_label(msg->order+1);
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),g_node_cnt); 
            msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),msg->node_id_des);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->part_id, 2),msg->node_id_des);
            msg_queue.enqueue(get_thd_id(),Message::create_message0(SET_DETEST, msg->part_id, 2),g_node_cnt);
        }
    #else
        //update_part_map(msg->part_id, msg->node_id_des);
        update_part_map_status(msg->part_id, 2);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),g_node_cnt);
        msg_queue.enqueue(get_thd_id(),Message::create_message1(SET_PARTMAP, msg->part_id, msg->node_id_des, 2),msg->node_id_des);
    #endif
#endif
    double migration_time = get_sys_clock() - start_time;
    std::cout<<"M Time:"<<migration_time / BILLION <<endl;
    g_mig_endtime = get_sys_clock();
    std::cout<<"Time:"<<(get_server_clock()-g_starttime) / BILLION<<endl;
    txn_man->txn_stats.migration_time = migration_time;
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
    txn_man->commit();
    return rc;
}