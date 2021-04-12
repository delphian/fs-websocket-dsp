/**
 * @file fs_ws_dsp_command.h
 * @brief Commands define what type of signal processing should be done on data.
 */

/**
 * Signal processing command type definitions.
 */
const static uint32_t FS_WS_DSP_COMMAND_ECHO = 1;

/**
 * @brief Signal processing to transform data with.
 */  
struct fs_ws_dsp_command {
    uint32_t type;     ///< Type of processing request.
    size_t params_len; ///< Length in bytes of parameters.
    char *params;      ///< Parameter data.
};

/**
 * @brief Free memory associated with signal processing command.
 * @param[in] command Signal processing command.
 */
struct fs_ws_dsp_command* fs_ws_dsp_command_free(struct fs_ws_dsp_command *command);

/**
 * @brief Parse signal processing command.
 * @details Parse binary stream of a signal processing command into fs_ws_dsp_command structure.
 * @param[in] stream Serialized signal processing command.
 */
struct fs_ws_dsp_command* fs_ws_dsp_command_parse(char *stream);

/**
 * @brief Serialize signal processing command to a single memory array.
 * @param[in] command Signal processing command.
 * @returns Signal processing command serialized into a memory array. 
 */
char* fs_ws_dsp_command_serialize(struct fs_ws_dsp_command *command);

/**
 * @brief Calculate size_t required to malloc a serialized form of signal processing command.
 * @param[in] command Signal processing command.
 */
size_t fs_ws_dsp_command_serialize_size(struct fs_ws_dsp_command *command);


