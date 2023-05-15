#include "mf_classic.h"

#include <furi/furi.h>
#include <lib/nfc/protocols/nfc_util.h>

typedef struct {
    uint8_t sectors_total;
    uint16_t blocks_total;
    const char* full_name;
    const char* type_name;
} MfClassicFeatures;

MfClassicFeatures mf_classic_features[MfClassicTypeNum] = {
    [MfClassicTypeMini] =
        {
            .sectors_total = 5,
            .blocks_total = 20,
            .full_name = "Mifare Classic Mini 0.3K",
            .type_name = "MINI",
        },
    [MfClassicType1k] =
        {
            .sectors_total = 16,
            .blocks_total = 64,
            .full_name = "Mifare Classic 1K",
            .type_name = "1K",
        },
    [MfClassicType4k] =
        {
            .sectors_total = 40,
            .blocks_total = 256,
            .full_name = "Mifare Classic 4K",
            .type_name = "4K",
        },
};

uint8_t mf_classic_get_total_sectors_num(MfClassicType type) {
    return mf_classic_features[type].sectors_total;
}

uint16_t mf_classic_get_total_block_num(MfClassicType type) {
    return mf_classic_features[type].blocks_total;
}

const char* mf_classic_get_name(MfClassicType type, bool full_name) {
    const char* ret = NULL;

    if(full_name) {
        ret = mf_classic_features[type].full_name;
    } else {
        ret = mf_classic_features[type].type_name;
    }

    return ret;
}

uint8_t mf_classic_get_sector_trailer_num_by_sector(uint8_t sector) {
    uint8_t block_num = 0;

    if(sector < 32) {
        block_num = sector * 4 + 3;
    } else if(sector < 40) {
        block_num = 32 * 4 + (sector - 32) * 16 + 15;
    } else {
        furi_crash("Wrong sector num");
    }

    return block_num;
}

uint8_t mf_classic_get_sector_trailer_num_by_block(uint8_t block) {
    uint8_t sec_tr_block_num = 0;

    if(block < 128) {
        sec_tr_block_num = block | 0x03;
    } else {
        sec_tr_block_num = block | 0x0f;
    }

    return sec_tr_block_num;
}

MfClassicSectorTrailer*
    mf_classic_get_sector_trailer_by_sector(MfClassicData* data, uint8_t sector_num) {
    furi_assert(data);

    uint8_t sec_tr_block = mf_classic_get_sector_trailer_num_by_sector(sector_num);
    MfClassicSectorTrailer* sec_trailer = (MfClassicSectorTrailer*)&data->block[sec_tr_block];

    return sec_trailer;
}

bool mf_classic_is_sector_trailer(uint8_t block) {
    return block == mf_classic_get_sector_trailer_num_by_block(block);
}

uint8_t mf_classic_get_sector_by_block(uint8_t block) {
    uint8_t sector = 0;

    if(block < 128) {
        sector = (block | 0x03) / 4;
    } else {
        sector = 32 + ((block | 0x0f) - 32 * 4) / 16;
    }

    return sector;
}

bool mf_classic_is_key_found(MfClassicData* data, uint8_t sector_num, MfClassicKeyType key_type) {
    furi_assert(data);

    bool key_found = false;
    if(key_type == MfClassicKeyTypeA) {
        key_found = (FURI_BIT(data->key_a_mask, sector_num) == 1);
    } else if(key_type == MfClassicKeyTypeB) {
        key_found = (FURI_BIT(data->key_b_mask, sector_num) == 1);
    }

    return key_found;
}

void mf_classic_set_key_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type,
    uint64_t key) {
    furi_assert(data);

    uint8_t key_arr[6] = {};
    MfClassicSectorTrailer* sec_trailer =
        mf_classic_get_sector_trailer_by_sector(data, sector_num);
    nfc_util_num2bytes(key, 6, key_arr);
    if(key_type == MfClassicKeyTypeA) {
        memcpy(sec_trailer->key_a.data, key_arr, sizeof(MfClassicKey));
        FURI_BIT_SET(data->key_a_mask, sector_num);
    } else if(key_type == MfClassicKeyTypeB) {
        memcpy(sec_trailer->key_b.data, key_arr, sizeof(MfClassicKey));
        FURI_BIT_SET(data->key_b_mask, sector_num);
    }
}

void mf_classic_set_key_not_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKeyType key_type) {
    furi_assert(data);

    if(key_type == MfClassicKeyTypeA) {
        FURI_BIT_CLEAR(data->key_a_mask, sector_num);
    } else if(key_type == MfClassicKeyTypeB) {
        FURI_BIT_CLEAR(data->key_b_mask, sector_num);
    }
}

bool mf_classic_is_block_read(MfClassicData* data, uint8_t block_num) {
    furi_assert(data);

    return (FURI_BIT(data->block_read_mask[block_num / 32], block_num % 32) == 1);
}

void mf_classic_set_block_read(MfClassicData* data, uint8_t block_num, MfClassicBlock* block_data) {
    furi_assert(data);

    if(mf_classic_is_sector_trailer(block_num)) {
        memcpy(&data->block[block_num].data[6], &block_data->data[6], 4);
    } else {
        memcpy(data->block[block_num].data, block_data->data, MF_CLASSIC_BLOCK_SIZE);
    }
    FURI_BIT_SET(data->block_read_mask[block_num / 32], block_num % 32);
}
