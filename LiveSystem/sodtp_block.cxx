#include <sodtp_block.h>
#include <util_log.h>

int BlockDataBuffer::write(uint32_t id, uint8_t *src, int size) {
    int ret = 0;
    timeFramePlayer.evalTimeStamp("buffer_write","p",std::to_string(id));
    // Find existing block, and push back the data.
    for (const auto &it : buffer) {
        if (it->id == id) {
            // timeFramePlayer.evalTimeStamp("buffer_write","p",std::to_string(id));
            return it->write(src, size);
        }
    }

    // Else, this is a new block.
    // Create new BlockData.
    BlockDataPtr ptr(new BlockData(id));
    buffer.push_front(ptr);

    // If too many blocks, pop the stale block.
    if (buffer.size() > MAX_BLOCK_NUM) {
        buffer.pop_back();
    }

    ret = ptr->write(src, size);

    return ret;
}

SodtpBlockPtr BlockDataBuffer::read(uint32_t id, SodtpStreamHeader *head) {
    SodtpBlockPtr ptr = NULL;
    int32_t size;
    uint8_t *data;
    // printf("buffer number = %d, block_id = %d\n", buffer.size(), block_id);
    for (const auto &it : buffer) {
        // printf("block id = %d,\t size = %d\n", it->block_id, it->offset - it->data);
        if (it->id == id) {
            // Print2File("if (it->id == id))=======================");
            ptr = std::make_shared<SodtpBlock>();

            // printf("data size = %d\n", it->offset - it->data);
            // printf("debug: %s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

            memcpy(head, it->data, sizeof(*head));
            data = it->data + sizeof(*head);
            size = it->offset - it->data - sizeof(*head);
            
            // 留作以后要传输类似context的东西
            // memcpy(context, it->data+sizeof(*head), sizeof(*context)); //原本
            // data = it->data + sizeof(*head) + sizeof(*context);
            // size = it->offset - it->data - sizeof(*head) - sizeof(*context);
            

            // Print2File("======================!!!=======================");
            // Print2File(" it->offset - it->data : "+std::to_string(it->offset - it->data));
            // Print2File(" sizeof(*head) : "+std::to_string(sizeof(*head)));
            // Print2File(" ======== size : "+std::to_string(size));
            // Print2File("ptr->last_one = (bool)(head->flag & HEADER_FLAG_FIN);");

            ptr->last_one = (bool)(head->flag & HEADER_FLAG_FIN);
            ptr->key_block = (bool)(head->flag & HEADER_FLAG_KEY);
            if((head->flag & HEADER_FLAG_KEY)){
                // Print2File("(head->flag & HEADER_FLAG_KEY)");
                //  int ret2 = lhs_copy_myParameters_to_myParameters(ptr->codecParPtr,&(head->codecPar));
                //  除了codecParExtradata的数据
                ptr->codecParPtr = &(head->codecPar);
                //  codecParExtradata的数据
                ptr->codecParExtradata = new uint8_t[size + AV_INPUT_BUFFER_PADDING_SIZE];
                memcpy(ptr->codecParExtradata, data, size);
                memset(ptr->codecParExtradata + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
                // Print2File("sizeof(*ptr->codecParExtradata) : "+std::to_string(sizeof(*ptr->codecParExtradata)));
                data = it->data + sizeof(*head) + sizeof(*(ptr->codecParExtradata));
                size = it->offset - it->data - sizeof(*head)-sizeof(*(ptr->codecParExtradata));
                // Print2File(" ======== size 2222 : "+std::to_string(size));
            }
            if(head->haveFormatContext){
                ptr->haveFormatContext = true;
            }
            ptr->stream_id = head->stream_id;
            ptr->block_ts = head->block_ts;
            ptr->block_id = head->block_id;
            ptr->duration = head->duration;
            
            // Print2File("ptr->data = new uint8_t[size + AV_INPUT_BUFFER_PADDING_SIZE];");
            // Print2File("new uint8_t[size + AV_INPUT_BUFFER_PADDING_SIZE] size : "+std::to_string(size));
            ptr->data = new uint8_t[size + AV_INPUT_BUFFER_PADDING_SIZE];
            // Print2File("ptr->data = new uint8_t[size + AV_INPUT_BUFFER_PADDING_SIZE];");
            ptr->size = size;
            // Print2File("ptr->size = size;");
            memcpy(ptr->data, data, size); 
            // Print2File("memcpy(ptr->data, data, size); ");
            memset(ptr->data + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
            // Print2File("memset break");
            timeFramePlayer.evalTimeStamp("buffer_read","p",std::to_string(ptr->block_id));
            break;
        }
    }
    return ptr;
}




