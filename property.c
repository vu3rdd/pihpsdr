/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include "property.h"
#include <stdio.h>   // for NULL, fclose, fopen, sprintf, fgets, fwrite, FILE
#include <stdlib.h>  // for malloc, free, atof
#include <string.h>  // for strlen, strcpy, strcmp, strtok
#include "log.h"     // for log_error

PROPERTY* properties=NULL;

static double version=0.0;

void clearProperties() {
    if(properties!=NULL) {
	// free all the properties
	PROPERTY *next;
	while(properties!=NULL) {
	    next=properties->next_property;
	    free(properties);
	    properties=next;
	}
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief Load Properties
*
* @param filename
*/
void loadProperties(char* filename) {
    char string[256];
    char* name;
    char* value;
    FILE* f=fopen(filename,"r");
    PROPERTY* property;

    clearProperties();
    if(f) {
        while(fgets(string,sizeof(string),f)) {
            if(string[0]!='#') {
                name=strtok(string,"=");
                value=strtok(NULL,"\n");
		// Beware of "illegal" lines in corrupted files
		if (name != NULL && value != NULL) {
                  property=malloc(sizeof(PROPERTY));
                  property->name=malloc(strlen(name)+1);
                  strcpy(property->name,name);
                  property->value=malloc(strlen(value)+1);
                  strcpy(property->value,value);
                  property->next_property=properties;
                  properties=property;
                  if(strcmp(name,"property_version")==0) {
                    version=atof(value);
                  }
		}
            }
        }
        fclose(f);
    }

    if(version!=PROPERTY_VERSION) {
      properties=NULL;
      log_error("loadProperties: version=%f expected version=%f ignoring",version,PROPERTY_VERSION);
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief Save Properties
*
* @param filename
*/
void saveProperties(char* filename) {
    PROPERTY* property;
    FILE* f=fopen(filename,"w+");
    char line[512];

    if(!f) {
        log_error("can't open %s",filename);
        return;
    }

    sprintf(line,"%0.2f",PROPERTY_VERSION);
    setProperty("property_version",line);
    property=properties;
    while(property) {
        sprintf(line,"%s=%s\n",property->name,property->value);
        fwrite(line,1,strlen(line),f);
        property=property->next_property;
    }
    fclose(f);
}

/* --------------------------------------------------------------------------*/
/**
* @brief Get Properties
*
* @param name
*
* @return
*/
char* getProperty(char* name) {
    char* value=NULL;
    PROPERTY* property=properties;
    while(property) {
        if(strcmp(name,property->name)==0) {
            value=property->value;
            break;
        }
        property=property->next_property;
    }
    return value;
}

/* --------------------------------------------------------------------------*/
/**
* @brief Set Properties
*
* @param name
* @param value
*/
void setProperty(char* name,char* value) {
    PROPERTY* property=properties;
    while(property) {
        if(strcmp(name,property->name)==0) {
            break;
        }
        property=property->next_property;
    }
    if(property) {
        // just update
        free(property->value);
        property->value=malloc(strlen(value)+1);
        strcpy(property->value,value);
    } else {
        // new property
        property=malloc(sizeof(PROPERTY));
        property->name=malloc(strlen(name)+1);
        strcpy(property->name,name);
        property->value=malloc(strlen(value)+1);
        strcpy(property->value,value);
        property->next_property=properties;
        properties=property;
    }
}

