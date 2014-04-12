/*
** Designation:  StriStr
**
** Call syntax:  char *stristr(char *String, char *Pattern)
**
** Description:  This function is an ANSI version of strstr() with
**               case insensitivity.
**
** Return item:  char *pointer if Pattern is found in String, else
**               pointer to 0
**
** Rev History:  16/07/97  Greg Thayer  Optimized
**               07/04/95  Bob Stout    ANSI-fy
**               02/03/94  Fred Cole    Original
**               09/01/03  Bob Stout    Bug fix (lines 40-41) per Fred Bulback
**
** Hereby donated to public domain.
*/
                   
#ifndef _STRISTR_H_
#define _STRISTR_H_

#ifdef __cplusplus
extern "C" {
#endif

char *stristr(const char *String, const char *Pattern);

#ifdef __cplusplus
}
#endif

#endif


