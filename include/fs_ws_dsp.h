/**
 * @file fs_ws_dsp.h
 * @brief Websocket server subprotocol for digital signal processing requests
 */

/**
 * @brief Signal processing request container.
 * @details This container will be created from parsing the client request.
 */
struct fs_ws_dsp_request {
    uint8_t _version;   ///< Version of the request format.
    uint32_t id;        ///< Each request must have a unique tracking identifier.
    size_t data_len;    ///< Byte length of data to be processed.
    void *data;         ///< Data specific to the processing request.
};

/**
 * @brief Signal processing response container.
 * @details This container will be serialized and returned to caller.
 */
struct fs_ws_dsp_response {
    uint8_t _version;   ///< Version of the response format.
    uint32_t id;        ///< Response is associated with request id.
    size_t data_len;    ///< Byte length of processed data.
    char *data;         ///< Data specific to the processing request.
};

/**
 * @brief Process signal processing request.
 * @param[in] request Signal processing request.
 */
struct fs_ws_dsp_response fs_ws_dsp_process(struct fs_ws_dsp_request request);

/**
 * @brief Free memory associated with signal processing response.
 * @param[in] response Signal processing response.
 */
void fs_ws_dsp_free_response(struct fs_ws_dsp_response response);

/**
 * @brief Parse processing request.
 * @details Parse binary stream of processing request into fs_ws_dsp_request structure.
 * @param[in] data Signal processing request.
 * @param[in] data_len Byte length of request data.
 */
struct fs_ws_dsp_request fs_ws_dsp_parse_request(char *data, size_t data_len);

/**
 * @brief Free memory associated with signal processing request.
 * @param[in] request Signal processing request.
 */
void fs_ws_dsp_free_request(struct fs_ws_dsp_request request);

/**
 * @brief Serialize signal processing response to a single memory array.
 * @param[in] response Signal processing response.
 */
char *fs_ws_dsp_serialize_response(struct fs_ws_dsp_response);

/**
 * @brief Calculate size_t required to malloc a serialized form of signal processing response.
 * @param[in] response Signal processing response.
 */
size_t fs_ws_dsp_get_response_size(struct fs_ws_dsp_response);
