#include "mre_ini_parser.h"

int filledDsplByLines = 0;
int entry_count = 0;
VMBOOL trigeris = VM_FALSE;   //is loaded ini file ?
VMBOOL mtrigeris = VM_FALSE;  //is need free malloc ?

VMINT		layer_hdl[1];				////layer handle array.
VMUINT8* screenbuf = NULL;	
VMINT ncharacter_height = -1;
VMINT nscreen_width = -1;
VMINT nscreen_height = -1;

struct configparser cfg;
struct config_entry entries[CONFIG_MAX_ENTRIES];

void vm_main(void) {

	vm_reg_sysevt_callback(handle_sysevt);
	vm_reg_keyboard_callback(handle_keyevt);
        vm_font_set_font_size(VM_SMALL_FONT);
        autoload_ini_file();

        ncharacter_height = vm_graphic_get_character_height();
        nscreen_width = vm_graphic_get_screen_width();
        nscreen_height = vm_graphic_get_screen_height();
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
	        screenbuf = vm_graphic_get_layer_buffer(layer_hdl[0]);
                //fill_white();
                if (trigeris == VM_FALSE) {
                   show_all();
                } else {
                   //display_scrolling_line("Please load ini file.");
                   draw_text_muiltline(screenbuf, "Please load ini file.", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);
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
           if(filledDsplByLines != 0) {
             draw_text_muiltline(screenbuf, "", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);
           }
           show_value("hello", "aaa");
        } else {
           draw_text_muiltline(screenbuf, "Not loaded ini file !", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);
        }

    }

    if (event == VM_KEY_EVENT_UP && keycode == VM_KEY_NUM2) {

        if (trigeris == VM_FALSE) {
           if(filledDsplByLines != 0) {
           draw_text_muiltline(screenbuf, "", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);
           }
           show_value("", "hello");
        } else {
           draw_text_muiltline(screenbuf, "Not loaded ini file !", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);
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
        //filledDsplByLines = 0;

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
            draw_text_muiltline(screenbuf, a, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);
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

        draw_text_muiltline(screenbuf, c, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);

        const char *v = find_config_value(entries, entry_count, section, name);
        if (v) {
           if(strlen(section) == 0) {
             sprintf(b, "%s = \"%s\"", name, v);
           } else {
             sprintf(b, "[%s] %s = \"%s\"", section, name, v);
           }

           draw_text_muiltline(screenbuf, b, 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);

        } else {

           draw_text_muiltline(screenbuf, "Paramether not found !", 0, filledDsplByLines, nscreen_width, nscreen_height, 2, 1, VM_COLOR_BLACK);   
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

    vm_file_seek(fp, 0, BASE_END);
    config->length = vm_file_tell(fp);
    vm_file_seek(fp, 0, BASE_BEGIN);

    if (mtrigeris == VM_TRUE) {
        vm_free(config->data);
        config->data = NULL;
        mtrigeris = VM_FALSE;
    }

    config->data = vm_malloc(config->length + 1);
    if (!config->data) {
        vm_file_close(fp);
        return 0;
    }

    vm_file_read(fp, config->data, config->length, &nread);
    config->data[nread] = '\0';

    vm_file_close(fp);

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

VMINT get_string_width(VMWCHAR *whead,VMWCHAR *wtail) {

	VMWCHAR * wtemp = NULL;
	VMINT width = 0;
	if (whead == NULL || wtail == NULL)
	return 0;
	wtemp = (VMWCHAR *)vm_malloc((wtail-whead)*2+2);
	
	if (wtemp == NULL)
	return 0;
	memset(wtemp, 0, (wtail-whead) * 2 + 2);
	memcpy(wtemp, whead, (wtail-whead) * 2);

	width = vm_graphic_get_string_width(wtemp);
	vm_free(wtemp);
	return width;
}

void draw_text_muiltline(VMUINT8 *disp_buf, VMSTR str, VMINT x, VMINT y, VMINT width, VMINT height, VMINT betlines, VMINT startLine, VMINT color) {

	VMWCHAR *ucstr;
	VMWCHAR *ucshead;
	VMWCHAR *ucstail;
	VMINT is_end = FALSE;
	VMINT nheight = y; 
	VMINT nline_height ;
	VMINT nlines = 0;

        if (y = 0) {fill_white();}

	if (str == NULL||disp_buf==NULL||betlines < 0) {return;}

	nline_height = vm_graphic_get_character_height() + betlines;
	ucstr = (VMWCHAR*)vm_malloc(2*(strlen(str)+1));

	if (ucstr == NULL) {return;}
	
	if(0 != vm_ascii_to_ucs2(ucstr,2*(strlen(str)+1),str)) {
		vm_free(ucstr);
		return ;
	}
	ucshead = ucstr;
	ucstail = ucshead + 1;
	
	while(is_end == FALSE)
	{
		//if (nheight+nline_height>y+height) // paliekam paskutine ekrano eilute del meniu
		if (nheight > y + height) { // jeigu uzimtas ekrano aukstis virsijo 320
                        fill_white();
                        nheight = 0;
			//break;
                }

		while (1)
		{
			if (get_string_width(ucshead,ucstail)<=width)
			{
				ucstail ++;
			}
			else
			{
				nlines++;
				ucstail --;
				break;
			}
			if (0==vm_wstrlen(ucstail))
			{
				is_end = TRUE;
				nlines++;
				break;
			}
		}
		if ( nlines >= startLine)
		{
			vm_graphic_textout(disp_buf, x, nheight, ucshead, (ucstail-ucshead), (VMUINT16)(color));
                        vm_graphic_flush_layer(layer_hdl, 1);
			nheight += nline_height;
                        filledDsplByLines = nheight;
		}
		ucshead = ucstail;
		ucstail ++;
	}
	vm_free(ucstr);
}
