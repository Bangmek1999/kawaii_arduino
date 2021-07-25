#ifndef HtmlPage_h
#define HtmlPage_h
#include "Arduino.h"

//char Index[]="<html><body><p>Welcome to my project</p><p>Please login:</p><form action=\"/login\" method=\"post\"><label for=\"fname\">user:</label> <input name=\"user\" type=\"text\" /><br /><br /> <label for=\"lname\">password:</label> <input name=\"password\" type=\"password\" /><br /><br /> <input type=\"submit\" value=\"Login\" /></form></body></html>";
char Index[]="<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head><body style=background-color:#afdbd2><h1>Welcome to Plant Kawaii</h1><p>Please login:</p><form action=\"/login\" method=\"post\"><label for=\"fname\">user:</label> <input name=\"user\" type=\"text\" /><br /><br /> <label for=\"lname\">password:</label> <input name=\"password\" type=\"password\" /><br /><br /> <input type=\"submit\" value=\"Login\" /></form></body></html>";
char Login_fail[]="<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head><body style=background-color:#afdbd2><h1>Welcome to Plant Kawaii</h1><br>The user or password is not correct!, please try again<br><p>Please login:</p><form action=\"/login\" method=\"post\"><label for=\"fname\">user:</label> <input name=\"user\" type=\"text\" /><br /><br /> <label for=\"lname\">password:</label> <input name=\"password\" type=\"password\" /><br /><br /> <input type=\"submit\" value=\"Login\" /></form></body></html>";

char HTML_header[]="";
char HTML_footer[]="";
#endif
