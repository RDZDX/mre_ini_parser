#ifndef _VRE_APP_WIZARDTEMPLATE_
#define	_VRE_APP_WIZARDTEMPLATE_

#include "vmsys.h"
#include "vmio.h"
#include "vmgraph.h"
#include "vmchset.h"
#include "vmstdlib.h"
#include "string.h"	

#define CONFIG_MAXLINE 256 //512; 4096
#define CONFIG_MAX_ENTRIES 100

VMINT		layer_hdl[1];

struct configparser {
    char *data;
    size_t length;
    size_t pos;
    char section[CONFIG_MAXLINE];
};

struct config_entry {
    char section[CONFIG_MAXLINE];
    char name[CONFIG_MAXLINE];
    char value[CONFIG_MAXLINE];
};

void handle_sysevt(VMINT message, VMINT param);
void handle_keyevt(VMINT event, VMINT keycode);
int configparser_parse(struct configparser *config, char *section, char *name, char *value);
void configparser_free(struct configparser *config);
void trim(char *s);
const char *find_config_value(struct config_entry *entries, int count, const char *section, const char *name);
void unload_config(void);
VMINT job(VMWCHAR *file_path, VMINT wlen);
static void fill_white(void);
static void autoload_ini_file(void);
static void show_all(void);
static void show_value(const char *section, const char *name);
void create_selfapp_txt_filename(VMWSTR text, VMSTR extt);
int configparser_load_file(struct configparser *config, VMWSTR filename);
int load_data_to_struct(struct configparser *config);
void draw_text_muiltline(VMUINT8 *disp_buf, VMSTR str, VMINT x, VMINT y, VMINT width, VMINT height,VMINT betlines,VMINT startLine, VMINT color);
VMINT get_string_width(VMWCHAR *whead,VMWCHAR *wtail);

#endif

