/**
 * @file fs_ws_dsp_message.h
 * @brief Websocket server subprotocol for digital signal processing messages
 */

/**
 * @brief Signal processing message.
 * @details Messages are exchanged between the client and server.
 */
struct fs_ws_dsp_message {
    uint8_t _version;                    ///< Version of the request format.
    uint32_t id;                         ///< Each request must have a unique tracking identifier.
    uint32_t commands_count;             ///< Number of commands.
    struct fs_ws_dsp_command **commands; ///< Pointer to array of command pointers.
    uint32_t data_len;                   ///< Byte length of data to be processed.
    void *data;                          ///< Data specific to the processing request.
};

/**
 * @brief Parse message.
 * @details Parse binary stream of message into fs_ws_dsp_message structure.
 * @param[in] data Signal processing message.
 * @param[in] data_len Byte length of message data.
 */
struct fs_ws_dsp_message fs_ws_dsp_message_parse(char *data, size_t data_len);

/**
 * @brief Calculate size_t required to malloc a serialized form of signal processing message.
 * @param[in] message Signal processing message.
 */
size_t fs_ws_dsp_message_serialize_size(struct fs_ws_dsp_message);

/**
 * @brief Serialize signal processing message structure to a single memory array.
 * @param[in] message Signal processing message.
 */
char *fs_ws_dsp_message_serialize(struct fs_ws_dsp_message);

/**
 * @brief Free memory associated with signal processing message.
 * @param[in] message Signal processing request.
 */
void fs_ws_dsp_message_free(struct fs_ws_dsp_message message);
