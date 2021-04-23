/**
 * @file fs_ws_dsp_message.c
 * @brief Websocket server subprotocol for digital signal processing messages
 */

struct fs_ws_dsp_message fs_ws_dsp_message_parse(char *data, size_t data_len) {
    struct fs_ws_dsp_message message;
    memset(&message, 0, sizeof(struct fs_ws_dsp_message));
    char *src = data;

// fs_ws_dsp_debug(data, 64);

    message._version       = *((uint8_t *)src);     src += 1;
    message.id             = *((uint32_t *)src);    src += 4;
    message.commands_count = *((uint32_t *)src);    src += 4;
    if (message.commands_count > 0) {
        message.commands = malloc(sizeof(struct fs_ws_dsp_command *) *message.commands_count);
        struct fs_ws_dsp_command **dst = message.commands;
        for (int i = 0; i < message.commands_count; i++) {
            struct fs_ws_dsp_command *command = fs_ws_dsp_command_parse(src);
            *dst++ = command;
            src += fs_ws_dsp_command_serialize_size(command);
        }
    }
    message.data_len = *((uint32_t *)src);          src += 4;
    if (message.data_len > 0) {
        message.data = malloc(message.data_len);
        memcpy(message.data, src, message.data_len);
    }
    return message;
}
void fs_ws_dsp_message_free(struct fs_ws_dsp_message message) {
	if (message.data != NULL)
		free(message.data);
    if (message.commands_count > 0) {
        struct fs_ws_dsp_command **command = message.commands;
        for (int i = 0; i < message.commands_count; i++)
            fs_ws_dsp_command_free(*(command++));
        free (message.commands);
    }
    memset(&message, 0, sizeof(struct fs_ws_dsp_message));
    return;
}
char *fs_ws_dsp_message_serialize(struct fs_ws_dsp_message message) {
    char *stream;
    char *dest;
    stream = malloc(fs_ws_dsp_message_serialize_size(message));
    dest = stream;
    memcpy(dest,                            &message._version, sizeof message._version);
    memcpy(dest += sizeof message._version, &message.id,       sizeof message.id);
    memcpy(dest += sizeof message.id,       &message.data_len, sizeof message.data_len);
    memcpy(dest += sizeof message.data_len, message.data,      message.data_len);
    return stream;
}
size_t fs_ws_dsp_message_serialize_size(struct fs_ws_dsp_message message) {
    return sizeof message._version +
           sizeof message.id + 
           sizeof message.data_len +
           message.data_len;
}
