/* strutils.c ; String Utility functions
 * Butter Utiltiy Library
 */

/* Licensed under the WTFPL version 2
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                    Version 2, December 2004
 *  
 * Copyright (C) 2023, Marie Eckert
 * Copyright (C) 2010-2022, ThhE <thhe@gmx.de>
 * 
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *  
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 * 
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 */

/* This library is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details. 
 */

#include <butter/strutils.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char str_terminator[] = "\0";
const char newline[] = "\n";

int str_startswith(char *str, char *start) {
  return strncmp(start, str, strlen(start));
}

int str_endswith(char *str, char *end) {
  int str_len = strlen(str);
  int end_len = strlen(end);

  return ((str_len > end_len) && !strcmp(str + str_len - end_len, end));
}

char *strcpy_until(char *src, char delimiter) {
  int offs = 0;
  while (offs < strlen(src)) {
    if (src[offs] == delimiter) break;
    offs++;
  }

  if (offs == 0)
    return "";

  char *res = malloc(offs+1);
  memcpy(res, src, offs);
  memcpy(res+offs, str_terminator, 1);

  return res;
}

char *bstrcpy_until(char *src, char *src_org, char delimiter) {
  int offs = 0;
  while ((src-offs) > src_org) {
    if ((src-offs)[0] == delimiter) break;
    offs++;
  }

  if (offs == 0)
    return "";

  char *res = malloc(offs+1);
  memcpy(res, src-offs+1, offs);
  memcpy(res+offs, str_terminator, 1);

  return res;
}

char *str_replace(char *orig, char *rep, char *with) {
  char *result; // the return string
  char *ins;    // the next insert point
  char *tmp;    // varies
  int len_rep;  // length of rep (the string to remove)
  int len_with; // length of with (the string to replace rep with)
  int len_front; // distance between rep and end of last rep
  int count;    // number of replacements

  // sanity checks and initialization
  if (!orig || !rep)
    return NULL;
  len_rep = strlen(rep);
  if (len_rep == 0)
    return NULL; // empty rep causes infinite loop during count
  if (!with)
    with = "";
  len_with = strlen(with);

  // count the number of replacements needed
  ins = orig;
  for (count = 0; (tmp = strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep; // move to next "end of rep"
  }

  strcpy(tmp, orig);
  return result;
}

char *trim_whitespace(char *str) {
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = str_terminator[0];

  return str;
}

char *strtok_asm_remain(char *delim) {
  char *token = strtok(NULL, delim);
  if (token == NULL) return NULL;

  char *result = strdup(token);

  token = strtok(NULL, delim);
  while (token != NULL) {
    result = realloc(result, strlen(result)+strlen(token)+2);
    strcpy(result+strlen(result), delim);
    strcpy(result+strlen(result), token);
    token = strtok(NULL, delim);
  }

  return result;
}

char *strglue(char *start, char *glue, char *end) {
  char *result = malloc(strlen(start)+strlen(glue)+strlen(end)+1);
  strcpy(result, start);
  strcpy(result+strlen(result), glue);
  strcpy(result+strlen(result), end);

  return result;
}
