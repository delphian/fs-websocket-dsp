/**
 * @file fs_ws_dsp_command.c
 * @brief Commands define what type of signal processing should be done on data.
 */

struct fs_ws_dsp_command* fs_ws_dsp_command_parse(char *stream) {
    struct fs_ws_dsp_command *command = malloc(sizeof(struct fs_ws_dsp_command));
    memset(command, 0, sizeof(struct fs_ws_dsp_command));
    command->type = *((uint32_t *)stream);
    command->params_len = *((uint32_t *)(stream + 4));
    if (command->params_len > 0) {
        command->params = malloc(command->params_len);
        memcpy(command->params, stream + 8, command->params_len);
    }
    return command;
}
struct fs_ws_dsp_command* fs_ws_dsp_command_free(struct fs_ws_dsp_command *command) {
	if (command->params != NULL)
		free(command->params);
    free(command);
    return 0;
}
char* fs_ws_dsp_command_serialize(struct fs_ws_dsp_command *command) {
    char *stream = malloc(fs_ws_dsp_command_serialize_size(command));
    char *dst = stream;
    memcpy(dst, &command->type,       sizeof command->type);        dst += sizeof(command->type);
    memcpy(dst, &command->params_len, sizeof command->params_len);  dst += sizeof(command->params_len);
    memcpy(dst, command->params,      command->params_len);
    return stream;
}
size_t fs_ws_dsp_command_serialize_size(struct fs_ws_dsp_command *command) {
    return sizeof command->type +
           sizeof command->params_len + 
           command->params_len;
}
