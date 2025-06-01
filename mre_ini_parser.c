#include "mre_ini_parser.h"

int filledDsplByLines = 0;
int entry_count = 0;
VMBOOL trigeris = VM_FALSE;   //is loaded ini file ?
VMBOOL mtrigeris = VM_FALSE;  //is need free malloc ?

struct configparser cfg;
struct config_entry entries[CONFIG_MAX_ENTRIES];

void vm_main(void) {

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
        vm_font_set_font_size(VM_SMALL_FONT);
        autoload_ini_file();
}

void handle_sysevt(VMINT message, VMINT param) {

	switch (message) {
	case VM_MSG_CREATE:
	case VM_MSG_ACTIVE:
		layer_hdl[0] = vm_graphic_create_layer(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height(), -1);	
		vm_graphic_set_clip(0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height());
                fill_white();
		break;

	case VM_MSG_PAINT:
                vm_switch_power_saving_mode(turn_off_mode);
                //fill_white();
                if (trigeris == VM_FALSE) {
                   show_all();
                } else {
                   display_scrolling_line("Please load ini file.");
                }
		break;
		
	case VM_MSG_INACTIVE:
                vm_switch_power_saving_mode(turn_on_mode);
		if( layer_hdl[0] != -1 )
			vm_graphic_delete_layer(layer_hdl[0]);
		
		break;	
	case VM_MSG_QUIT:
		if( layer_hdl[0] != -1 )
			vm_graphic_delete_layer(layer_hdl[0]);
		
		break;	
	}
}

void handle_keyevt(VMINT event, VMINT keycode) {

    if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_RIGHT_SOFTKEY) {
        if (layer_hdl[0] != -1) {
            vm_graphic_delete_layer(layer_hdl[0]);
            layer_hdl[0] = -1;
        }
        vm_exit_app();
    }

    if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_LEFT_SOFTKEY) {

        vm_selector_run(0, 0, job);
    }

    if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_NUM1) {

        if (trigeris == VM_FALSE) {
           if(filledDsplByLines != 0) {display_scrolling_line("");}
           show_value("hello", "aaa");
        } else {
           display_scrolling_line("Not loaded ini file !");
        }

    }

    if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_NUM2) {

        if (trigeris == VM_FALSE) {
           if(filledDsplByLines != 0) {display_scrolling_line("");}
           show_value("", "hello");
        } else {
           display_scrolling_line("Not loaded ini file !");
        }

    }

}

int configparser_parse(struct configparser *config, char *section, char *name, char *value) {

    char line[CONFIG_MAXLINE];
    size_t i, len;
    char *t;

    memset(name, 0, CONFIG_MAXLINE);
    memset(value, 0, CONFIG_MAXLINE);
    memset(section, 0, CONFIG_MAXLINE);

    while (config->pos < config->length) {
        i = 0;

        while (config->pos < config->length && config->data[config->pos] != '\n') {
            if (i < CONFIG_MAXLINE - 1) {
                line[i++] = config->data[config->pos];
            }
            config->pos++;
        }

        if (config->pos < config->length && config->data[config->pos] == '\n') {
            config->pos++;
        }

        line[i] = '\0';

        len = strlen(line);
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
            len--;
        }

        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#' || *p == ';') continue;

        if (*p == '[' && p[len - 1] == ']') {
            sscanf(p, "[%[^]]", config->section);
            continue;
        }

        char *sep = strchr(p, '=');
        if (!sep) sep = strchr(p, ':');
        if (!sep) continue;

        *sep = '\0';
        char *key = p;
        char *val = sep + 1;

        while (*val == ' ' || *val == '\t') val++;

        int in_quote = 0;
        char quote_char = 0;
        for (t = val; *t; t++) {
            if ((*t == '"' || *t == '\'') && (t == val || *(t - 1) != '\\')) {
                if (in_quote && *t == quote_char) {
                    in_quote = 0;
                } else if (!in_quote) {
                    in_quote = 1;
                    quote_char = *t;
                }
            } else if (!in_quote && (*t == ';' || *t == '#')) {
                *t = '\0';
                break;
            } else if (*t == '\t') {
                *t = ' ';
            }
        }

        trim(key);
        trim(val);

        size_t vlen = strlen(val);
        if ((val[0] == '"' && val[vlen - 1] == '"') ||
            (val[0] == '\'' && val[vlen - 1] == '\'')) {
            val[vlen - 1] = '\0';
            val++;
            trim(val);
        }

        strcpy(name, key);
        strcpy(value, val);
        strcpy(section, config->section);

        return 1;
    }

    return 0;
}

void display_scrolling_line(VMSTR text_string) {

    VMWCHAR str[61];
    VMWCHAR xtest_wstring[8] = {0};
    VMINT xfont_height = 0;      //string "WWp" height --16
    VMINT xfont_baseline = 0;    //string "WWp" baseline --2
    vm_graphic_color color;

    vm_ascii_to_ucs2(xtest_wstring, 8, "WWp");  //Š???
    xfont_baseline = vm_graphic_get_string_baseline(xtest_wstring);
    xfont_height = vm_graphic_get_string_height(xtest_wstring);

    vm_ascii_to_ucs2(str, (strlen(text_string) + 1) * 2, text_string);

    if (filledDsplByLines > vm_graphic_get_screen_height() || filledDsplByLines == 0) {
       //filledDsplByLines = filledDsplByLines + 1;
       filledDsplByLines = 0;
       color.vm_color_565 = VM_COLOR_WHITE;
       vm_graphic_setcolor(&color);
       vm_graphic_fill_rect_ex(layer_hdl[0], 0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height());
    }

    color.vm_color_565 = VM_COLOR_BLACK;
    vm_graphic_setcolor(&color);
    vm_graphic_textout_to_layer_by_baseline(layer_hdl[0], 0, filledDsplByLines, str, wstrlen(str), xfont_baseline);
    vm_graphic_flush_layer(layer_hdl, 1);
    filledDsplByLines += xfont_height;
}

void trim(char *str) {

    char *start = str;
    while (*start == ' ' || *start == '\t') start++;

    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')) end--;

    *(end + 1) = '\0';

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

const char *find_config_value(struct config_entry *entries, int count, const char *section, const char *name) {

    int i;

    for (i = 0; i < count; i++) {
        if (((section == NULL || section[0] == '\0') && entries[i].section[0] == '\0') || (section && strcmp(entries[i].section, section) == 0)) {
            
            if (strcmp(entries[i].name, name) == 0) {
                return entries[i].value;
            }
        }
    }
    return NULL;
}

void unload_config(void) {

    if (mtrigeris == VM_TRUE) {
        vm_free(cfg.data);
        cfg.data = NULL;
        mtrigeris = VM_FALSE;
    }

    cfg.length = 0;
    cfg.pos = 0;
    memset(cfg.section, 0, CONFIG_MAXLINE);

    memset(entries, 0, sizeof(entries));
    entry_count = 0;
}


VMINT job(VMWCHAR *file_path, VMINT wlen) {

    trigeris = VM_FALSE;
    unload_config();
    configparser_load_file(&cfg, file_path);
    load_data_to_struct(&cfg);
    filledDsplByLines = 0;

    return 0;
}

static void fill_white(void) {

	vm_graphic_color color;

	color.vm_color_565 = VM_COLOR_WHITE;
	vm_graphic_setcolor(&color);
	vm_graphic_fill_rect_ex(layer_hdl[0], 0, 0, vm_graphic_get_screen_width(), vm_graphic_get_screen_height());
	vm_graphic_flush_layer(layer_hdl, 1);
        filledDsplByLines = 0;

}

static void autoload_ini_file(void) {

        VMWCHAR ini_path[100] = {0};
        VMFILE f_read;

        create_selfapp_txt_filename(ini_path, "ini");

	f_read = vm_file_open(ini_path, MODE_READ, FALSE);

	if (f_read < 0) {
           vm_file_close(f_read);
           //vm_selector_run(0, 0, job);
           trigeris = VM_TRUE;
	} else {
           vm_file_close(f_read);
	   configparser_load_file(&cfg, ini_path);
           load_data_to_struct(&cfg);
	   trigeris = VM_FALSE;
	}
}

static void show_all(void) {

	VMCHAR a[300] = {0};
        int i;

	for (i = 0; i < entry_count; i++) {
            sprintf(a, "[%s] %s = %s\n", entries[i].section, entries[i].name, entries[i].value);
            display_scrolling_line(a);
	}
}

static void show_value(const char *section, const char *name) {

        VMCHAR b[300] = {0};
        VMCHAR c[300] = {0};

        if(strlen(section) == 0) {
          sprintf(c, "Value of \"%s\" without section", name);
        } else {
          sprintf(c, "Value of \"%s\" in section [%s]", name, section);
        }

        display_scrolling_line(c);

        const char *v = find_config_value(entries, entry_count, section, name);
        if (v) {
           if(strlen(section) == 0) {
             sprintf(b, "%s = \"%s\"", name, v);
           } else {
             sprintf(b, "[%s] %s = \"%s\"", section, name, v);
           }

           display_scrolling_line(b);

        } else {
           display_scrolling_line("Paramether not found !");
        }


}

void create_selfapp_txt_filename(VMWSTR text, VMSTR extt) {

    VMWCHAR fullPath[100] = {0};
    VMWCHAR wfile_extension[8] = {0};

    vm_get_exec_filename(fullPath);
    vm_ascii_to_ucs2(wfile_extension, 8, extt);
    vm_wstrncpy(text, fullPath, vm_wstrlen(fullPath) - 3);
    vm_wstrcat(text, wfile_extension);

}

int configparser_load_file(struct configparser *config, VMWSTR filename) {

    VMFILE fp;
    VMUINT nread;

    fp = vm_file_open(filename, MODE_READ, FALSE);
    if (fp < 0) {
        return 0;
    }

    // Nustatome failo ilgį
    vm_file_seek(fp, 0, BASE_END);
    config->length = vm_file_tell(fp);
    vm_file_seek(fp, 0, BASE_BEGIN);

    // Apsauga: jei atmintis buvo jau paskirta, ją atlaisviname
    if (mtrigeris == VM_TRUE) {
        vm_free(config->data);
        config->data = NULL;
        mtrigeris = VM_FALSE;
    }

    // Paskiriame naują atmintį
    config->data = vm_malloc(config->length + 1);
    if (!config->data) {
        vm_file_close(fp);
        return 0;
    }

    // Nuskaitome failo duomenis
    vm_file_read(fp, config->data, config->length, &nread);
    config->data[nread] = '\0';

    // Uždarome failą
    vm_file_close(fp);

    // Nustatome pradinę poziciją ir išvalome buvusią sekciją
    config->pos = 0;
    config->section[0] = '\0';

    return 1;
}

int load_data_to_struct(struct configparser *config) {

    while (entry_count < CONFIG_MAX_ENTRIES &&
           configparser_parse(config,
                              entries[entry_count].section,
                              entries[entry_count].name,
                              entries[entry_count].value)) {
        entry_count++;
    }

    return 0;

}