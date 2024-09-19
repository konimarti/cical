CICAL(7)

# NAME

cical - an iCalendar object parser

# SYNOPSIS

*cical* [*-h*] [*-v*] [*-j* | *-m*] [*-i* _<file>_] [*-o* _<file>_]

*cical*(7) parses any iCalendar object stream which can then be converted to
either the JSON or Markdown format.

# OPTIONS

*-h*
	Show help message.

*-v*
	Show version.

*-j*
	Print parsed iCalendar in JSON format.

*-m*
	Print parsed iCalendar in Markdown format.

*-i FILE*
	Read input from filename (default: stdin).

*-o FILE*
	Write output to filename (default: stdout).

# AUTHOR

Koni Marti
