#ifndef RESPONSES_H_
#define RESPONSES_H_

#define ERROR RED("500: Error")
#define PWD_SUCCESS(arg) GREEN("257: " + arg)
#define QUIT_SUCCESS GREEN("221: Successful Quit.")
#define CWD_SUCCESS GREEN("250: Successful change.")
#define LS_SUCCESS GREEN("‫‪226:‬‬ ‫‪List‬‬ ‫‪transfer‬‬ ‫‪done.‬‬")
#define BAD_SQ RED("503: Bad sequence of commands.")
#define NEED_LOGIN RED("332: Need account for login.")
#define FILE_UNAVAILABLE RED("550: File unavailable.")
#define RETR_SUCCESS GREEN("‫‪226:‬‬ ‫‪Successful‬‬ ‫‪Download.‬‬")
#define RENAME_SUCCESS GREEN("250: Successful change.")
#define DELE_SUCCESS(arg) GREEN("‫‪250:‬‬ " + arg + " deleted.")
#define MKDIR_SUCCESS(arg) GREEN("257: " + arg + " created.")
#define DATA_CAP_ERROR RED("425: Can't open data connection.")
#define USERNAME_OK GREEN("331: User name okay, need password")
#define INVALID_USERNAME RED("430: Invalid username or password")
#define INVALID_PASSWORD RED("430: Invalid username or password")
#define SYNTAX_ERROR RED("‫‪501:‬‬ ‫‪Syntax‬‬ ‫‪error‬‬ ‫‪in‬‬ ‫‪parameters‬‬ ‫‪or‬‬ ‫‪arguments.‬‬")
#define PASSWORD_OK GREEN("230: User logged in, proceed. Logged out if appropriate.")

#define RED(arg) (string("\033[1;31m") + arg + "\033[0m")
#define GREEN(arg) (string("\033[1;32m") + arg + "\033[0m")

#endif