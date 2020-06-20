#ifndef RETURN
#define RETURN

#define ASSERT_SUCCESS(x)   \
    if (x != RET_SUCCESS) { \
        return x;           \
    }

#define ASSERT_SUCCESS_TWI_STOP(x) \
    if (status != RET_SUCCESS) {   \
        twi_stop();                \
        return status;             \
    }

typedef enum {
    RET_SUCCESS,

    RET_OWI_NO_PRESENCE,
    RET_OWI_UNKNOWN_RES,
    RET_OWI_CRC_ERR,
    RET_OWI_INVALID_FAMILY_CODE,
    RET_OWI_SEARCH_LAST_DEVICE,

    RET_TWI_NO_ACK,
    RET_TWI_START_ERR,

    RET_PACKET_MINIMAL_LENGTH_ERR,
    RET_PACKET_LENGTH_MISMATCH,
    RET_PACKET_CRC_ERR,

    RET_PH_SYNTAX_ERR,
    RET_PH_NO_RESPONSE,

    RET_EC_SYNTAX_ERR,
    RET_EC_NO_RESPONSE

} return_status_t;
#endif /* RETURN */
