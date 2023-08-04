/*
 * mcfg_parse.c ; author: Marie Eckert
 *
 * mcfg parser and utilities.
 * This source file and its header originally stem from mariebuild and have
 * been partially rewritten to be used as a general configuration file.
 *
 * Copyright (c) 2023, Marie Eckert
 * Licensed under the BSD 3-Clause License
 * <https://github.com/FelixEcker/mcfg/blob/master/LICENSE>
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <mcfg.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <butter/strutils.h>

/******** file private ********/

static mcfg_stype strtostype(char *str) {
  if (strcmp(str, "fields") == 0)
    return ST_FIELDS;

  if (strcmp(str, "lines") == 0)
    return ST_LINES;

  return ST_UNKNOWN;
}

static mcfg_ftype strtoftype(char *str) {
  if (strcmp(str, "str") == 0)
    return FT_STRING;

  if (strcmp(str, "list") == 0)
    return FT_LIST;

  return FT_UNKNOWN;
}

/******** mcfg.h ********/

void free_mcfg_file(mcfg_file* file) {
  for (int i = 0; i < file->sector_count; i++) {
    for (int j = 0; j < file->sectors[i].section_count; j++) {
      for (int k = 0; k < file->sectors[i].sections[j].field_count; k++) {
        free(file->sectors[i].sections[j].fields[k].name);
        free(file->sectors[i].sections[j].fields[k].value);
      }

      if (file->sectors[i].sections[j].field_count > 0)
        free(file->sectors[i].sections[j].fields);
      free(file->sectors[i].sections[j].name);

      if (file->sectors[i].sections[j].lines != NULL)
        free(file->sectors[i].sections[j].lines);
    }

    free(file->sectors[i].sections);
    free(file->sectors[i].name);
  }

  free(file->sectors);
  free(file);
}

/* Parsing Functions */

int register_sector(struct mcfg_file* file, char *name) {
  if (name == NULL) return MCFG_ERR_UNKNOWN;

  // Check for duplicate sectors
  for (int i = 0; i < file->sector_count; i++)
    if (strcmp(file->sectors[i].name, name) == 0)
      return MCFG_PERR_DUPLICATE_SECTOR;

  int wi = file->sector_count;
  file->sector_count++;

  if (wi == 0) {
    file->sectors = malloc(sizeof(mcfg_sector));
  } else {
    file->sectors = realloc(file->sectors, (wi+1) * sizeof(mcfg_sector));
  }

  file->sectors[wi].section_count = 0;
  file->sectors[wi].name = strdup(name);

  return MCFG_OK;
}

int register_section(struct mcfg_sector* sector, mcfg_stype type, char *name) {
  if (name == NULL) return MCFG_ERR_UNKNOWN;

  // Check for duplicate sections
  for (int i = 0; i < sector->section_count; i++)
    if (strcmp(sector->sections[i].name, name) == 0)
      return MCFG_PERR_DUPLICATE_SECTION;

  int wi = sector->section_count;
  sector->section_count++;

  if (wi == 0) {
    sector->sections = malloc(sizeof(mcfg_section));
  } else {
    sector->sections = realloc(sector->sections, (wi+1) * sizeof(mcfg_section));
  }

  sector->sections[wi].field_count = 0;
  sector->sections[wi].lines = NULL;
  sector->sections[wi].name = strdup(name);
  sector->sections[wi].type = type;

  return MCFG_OK;
}

int register_field(struct mcfg_section* section, mcfg_ftype type,
                       char *name, char *value) {
  if (name == NULL || value == NULL) return MCFG_ERR_UNKNOWN;

  // Check for duplicate fields
  for (int i = 0; i < section->field_count; i++)
    if (strcmp(section->fields[i].name, name) == 0)
      return MCFG_PERR_DUPLICATE_FIELD;

  int wi = section->field_count;
  section->field_count++;

  if (wi == 0) {
    section->fields = malloc(sizeof(mcfg_field));
  } else {
    section->fields = realloc(section->fields, (wi+1) * sizeof(mcfg_field));
  }

  section->fields[wi].type = type;
  section->fields[wi].name = strdup(name);
  section->fields[wi].value = strdup(value);

  return MCFG_OK;
}

int parse_line(struct mcfg_file* file, char *line) {
  char delimiter[] = " ";
  line = trim_whitespace(line);
  char *token = strtok(line, delimiter);

  while (token != NULL) {
    if (strcmp(token, "sector") == 0) {
      token = strtok(NULL, delimiter);

      return register_sector(file, token);
    } else if (strncmp(";", token, strlen(";")) == 0) break;

    if (strcmp(token, "fields") == 0 || strcmp(token, "lines") == 0) {
      mcfg_stype type = strtostype(token);
      if (type == ST_UNKNOWN)
        return MCFG_PERR_INVALID_SYNTAX;

      token = strtok(NULL, delimiter);

      if (token == NULL)
        return MCFG_PERR_INVALID_SYNTAX;

      // Remove the colon at the end of the name
      token[strlen(token)-1] = 0;
      return register_section(&file->sectors[file->sector_count-1], 
                                type, token);
    }

    if (file->sector_count == 0)
      return MCFG_PERR_INVALID_SYNTAX;

    if (file->sectors[file->sector_count-1].section_count == 0)
      return MCFG_PERR_INVALID_SYNTAX;

    mcfg_sector *sector = &file->sectors[file->sector_count-1];
    mcfg_section *section = &sector->sections[sector->section_count-1];

    if (section->type == ST_FIELDS) {
      mcfg_ftype type = strtoftype(token);
      token = strtok(NULL, delimiter);
      if (token == NULL)
        return MCFG_PERR_INVALID_SYNTAX;

      char *name = token;
      char *content = strtok_asm_remain(delimiter);
      if(content == NULL)
        return MCFG_PERR_INVALID_SYNTAX;

      // Gets rid of the quotation marks around the value
      // TODO: Pontentially make this less cancerous
      char *cleaned_content = malloc(strlen(content)-1);
      memcpy(cleaned_content, content+1, strlen(content)-2);
      memcpy(cleaned_content+strlen(content)-2, str_terminator, 1);
      free(content);

      int ret = register_field(section, type, name, cleaned_content);
      free(cleaned_content);
      return ret;
    } else {
      // Assemble content
      char *first_word = strdup(token);
      char *remain = strtok_asm_remain(delimiter);
      char *content = NULL;
      if (remain != NULL) {
        content = strglue(first_word, delimiter, remain);
        free(first_word);
        free(remain);
      } else {
        content = first_word;
      }

      // Add missing newline
      content = realloc(content, strlen(content)+2);
      strcpy(content+strlen(content), newline);

      // Write content to section
      if (section->lines != NULL) {
        section->lines = realloc(section->lines,
                                 strlen(section->lines)+
                                 strlen(content)+1);
        strcpy(section->lines+strlen(section->lines), content);
      } else {
        section->lines = strdup(content);
      }

      free(content);
    }

    token = strtok(NULL, delimiter);
  }

  return MCFG_OK;
}

int parse_file(struct mcfg_file* build_file) {
  FILE *file;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  errno = 0;
  file = fopen(build_file->path, "r");
  if (file == NULL)
    return MCFG_ERR_MASK_ERRNO | errno;

  build_file->sector_count = 0;
  build_file->line = 0;

  while ((read = getline(&line, &len, file)) != -1) {
    build_file->line++;
    int result = parse_line(build_file, line);
    if (result != MCFG_OK)
      return result;
  }

  fclose(file);
  if (line)
    free(line);

  return MCFG_OK;
}

/* Navigation Functions */

/* NOTE: The returned string of this function has to be freed after usage!!!
 */
char *get_path_elem(char *path, int n_elem) {
  if (path == NULL)
    return NULL;

  char delimiter = '/';

  // We do this atrocity of searching through the string since
  // this is the only way I could reliably implement this func
  // without having memory leaks or invalid free's
  // If anyone can do this nicer without creating memory leaks
  // or invalid free's, feel free to do so. I have wasted to
  // much time on this function

  int n = 0;
  int i = 0;
  while (n < n_elem) {
    if (i == strlen(path)) return NULL;
    if (path[i] == delimiter) n++;
    i++;
  }

  // Get len
  int len = 1;

  while ((i+len) < strlen(path)) {
    if (path[i+len] == delimiter) break;

    len++;
  }

  char *result = malloc(len+1);
  memcpy(result, path+i, len);
  memcpy(result+len, str_terminator, 1);

  return result;
}

mcfg_sector *find_sector(struct mcfg_file* file, char *sector_name) {
  for (int i = 0; i < file->sector_count; i++)
    if (strcmp(file->sectors[i].name, sector_name) == 0)
      return &file->sectors[i];

  return NULL;
}

mcfg_section *find_section(struct mcfg_file* file, char *path) {
  char *sector_name = get_path_elem(path, 0);
  char *section_name = get_path_elem(path, 1);

  if ((sector_name == NULL) || (section_name == NULL))
    return NULL;

  mcfg_sector *sector = find_sector(file, sector_name);
  free(sector_name);

  if (sector == NULL) {
    return NULL;
  }

  mcfg_section *section = NULL;
  for (int i = 0; i < sector->section_count; i++) {
    if (strcmp(sector->sections[i].name, section_name) == 0) {
      section = &sector->sections[i];
      break;
    }
  }

  free(section_name);
  return section;
}

mcfg_field *find_field(struct mcfg_file* file, char *path) {
  char *sector_name = get_path_elem(path, 0);
  char *section_name = get_path_elem(path, 1);
  char *field_name = get_path_elem(path, 2);

  if ((sector_name == NULL) || (section_name == NULL) || (field_name == NULL))
    return NULL;

  sector_name = realloc(sector_name,
                        strlen(sector_name)+strlen(section_name)+2);
  mcfg_section *section = find_section(file,
                                     strcat(strcat(sector_name, "/"),
                                            section_name)
                                    );

  free(sector_name);
  free(section_name);
  if (section == NULL) {
    free(field_name);
    return NULL;
  }

  mcfg_field *field = NULL;

  for (int i = 0; i < section->field_count; i++) {
    if (strcmp(section->fields[i].name, field_name) == 0) {
      field = &section->fields[i];
      break;
    }
  }

  free(field_name);

  return field;
}

char *format_list_field(struct mcfg_file file, mcfg_field field, char *context,
                         char *in, int in_offs, int len) {
  if (in == NULL || strcmp(in, "") == 0) return "";
  char *prefix = bstrcpy_until(in+in_offs-1, in, ' ');
  char *postfix = strcpy_until(in+in_offs+len+1, ' ');
  int prefix_len = 0;
  int postfix_len = 0;

  if (prefix[strlen(prefix)-1] == ':' && field.type == LIST) {
    free(prefix);
    prefix = NULL;
  }

  if (postfix[0] == ':' && field.type == LIST) {
    free(postfix);
    postfix = NULL;
  }

  if (prefix != NULL)
    prefix_len = strlen(prefix);

  if (postfix != NULL)
    postfix_len = strlen(postfix);

  char delimiter[] = ":";
  char *field_cpy = resolve_fields(file, field.value, context);

  char *f_elem = strtok(field_cpy, delimiter);
  if (f_elem == NULL)
    return "";

  char *result = malloc(strlen(f_elem)+prefix_len+postfix_len+1);

  const int base_size = prefix_len+postfix_len+2;
  int offs = 0;
  while (f_elem != NULL) {
    printf("f_elem=%s\n", f_elem);
    if (offs > 0) {
      int size =
        strlen(result)+strlen(f_elem)+base_size;
      result = realloc(
                  result, size
                );
      memcpy(result+offs, " ", 1);
      offs++;
    }

    if (offs > 0 && prefix != NULL) {
      memcpy(result+offs, prefix, strlen(prefix));
      offs += strlen(prefix);
    }

    memcpy(result+offs, f_elem, strlen(f_elem));
    offs += strlen(f_elem);
    f_elem = strtok(NULL, delimiter);

    if (f_elem != NULL && postfix != NULL) {
      strcpy(result+offs, postfix);
      offs += strlen(postfix);
    }
  }

  memcpy(result+offs, str_terminator, 1);

  free(field_cpy);
  if (prefix != NULL && strcmp(prefix, "") != 0)
    free(prefix);
  if (postfix != NULL && strcmp(postfix, "") != 0)
    free(postfix);

  return result;
}

/* TODO: This is singlehandidly the worst code ive ever written, this needs a
 * desperate cleanup its so fucking long and confusing
 * */
char *resolve_fields(struct mcfg_file file, char *in, char *context) {
  int n_fields = 0;
  int *field_indexes = malloc(sizeof(int));
  int *field_lens    = malloc(sizeof(int));
  char **fieldvals   = malloc(sizeof(char*));

  // Resolve all fields and store their vals and indexes in the string
  for (int i = 0; i < strlen(in); i++) {
    if ((in[i] == '$') && (in[i+1] == '(')) {
      int len = 0;
      int is_local = 1;

      for (int j = i; j < strlen(in); j++) {
        if (in[j] == '/') is_local = 0;
        if (in[j] == ')') break;
        len++;
      }

      char *name;

      // terminator offset
      int term_offs = len-2;
      if (is_local) {
        // Subtract two from length to account for $() = -3
        // and the NULL Byte = +1
        name = malloc(strlen(context) + (len - 1));
        memcpy(name, context, strlen(context));
        memcpy(name+strlen(context), in+i+2, len-2);
        term_offs += strlen(context);
      } else {
        char prefix[] = ".config/";
        int offs = 0;

        // Create temporary copy of raw name for prefix checking
        char *tmp_name = malloc((len - 1));
        memcpy(tmp_name, in+i+2, len-2);
        memcpy(tmp_name+len-2, str_terminator, 1);

        if (str_startswith(tmp_name, prefix) != 0) {
          name = malloc(strlen(prefix) + (len - 1));
          memcpy(name, prefix, strlen(prefix));
          offs += strlen(prefix);
        } else {
          name = malloc(len-2);
        }

        memcpy(name+offs, in+i+2, len-2);
        term_offs += offs;
        free(tmp_name);
      }

      memcpy(name+term_offs, str_terminator, 1);

      mcfg_field *field = find_field(&file, name);
      if ((field == NULL) || (field->value == NULL))
        goto resolve_fields_stop;

      char *val_tmp;
      if (field->type == FT_LIST) {
        val_tmp = format_list_field(file, (*field), context, in, i, len);

      } else {
        val_tmp = resolve_fields(file, field->value, context);
      }

      printf("val_tmp=%s\n", val_tmp);

      n_fields++;
      if (n_fields > 1) {
        field_indexes = realloc(field_indexes, n_fields*sizeof(int));
        field_lens    = realloc(field_lens   , n_fields*sizeof(int));
        fieldvals     = realloc(fieldvals    , n_fields*sizeof(char*));
      }

      field_indexes[n_fields-1] = i;
      field_lens[n_fields-1]    = len;
      fieldvals[n_fields-1]     = val_tmp;

    resolve_fields_stop:
      free(name);
    }
  }

  char *out = NULL;
  if (n_fields == 0) {
    out = malloc(strlen(in)+1);
    strcpy(out, in);

    goto resolve_fields_finished;
  }

  // Copy input string and insert values
  int i_offs = 0; // Offset for copying from in
  int o_offs = 0; // Offset for copying to out

  for (int i = 0; i < n_fields; i++) {
    int len = field_lens[i];
    int ix  = field_indexes[i];
    char *val = fieldvals[i];

    // allocate memory (strlen(val) + ix-i_offs)
    if (out == NULL) {
      out = malloc(strlen(val) + (ix - i_offs));
    } else {
      out = realloc(out, o_offs + (strlen(val) + (ix - i_offs)));
    }

    // copy from in i_offs <-> ix to out with o_offs
    memcpy(out+o_offs, in + i_offs, ix - i_offs);

    // add ix - i_offs to o_offs
    o_offs += ix - i_offs;

    // set i_offs to ix+len
    i_offs = ix + len + 1;

    // copy val to o_offs
    memcpy(out+o_offs, val, strlen(val));

    // add strlen of val to o_offs
    o_offs += strlen(val);
  }

  // Copy remaining bytes from in to out
  if (i_offs < strlen(in)) {
    int missing = strlen(in) - i_offs;

    out = realloc(out, o_offs + missing);
    memcpy(out+o_offs, in+i_offs, missing);
    o_offs += missing;
  }

  // Append Terminator
  out = realloc(out, o_offs+1);
  memcpy(out+o_offs, str_terminator, 1);

resolve_fields_finished:
  free(field_indexes);
  free(field_lens);
  for (int i = 0; i < n_fields; i++)
    if (fieldvals[i] != NULL && strcmp(fieldvals[i], "") != 0)
      free(fieldvals[i]);
  free(fieldvals);

  return out;
}
