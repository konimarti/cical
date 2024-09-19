# cical

[RFC 5545](https://www.rfc-editor.org/rfc/rfc5545.txt) implementation in C to
parse iCalendar objects streams.

This is still work-in-progress.

## Usage

```
usage: cical [-h|-v] [-j] [-i FILE] [-o FILE]

Parse iCalendar streams.

options:
  -h       show this help message
  -v 	   show version
  -j 	   print in json format
  -m 	   print in markdown format
  -i FILE  read from filename (default stdin)
  -o FILE  write to filename (default stdout)
```

## TODOs

-   [x] parse any iCalendar object stream
-   [x] parse date time (local time, time with timezone, UTC time)
-   [ ] parse parameter list (key=value list)
-   [x] print in (basic) JSON format
-   [x] print in (basic) Markdown format

## Contributing

Send patches on the [mailing list] or create pull-requests on [Github].

## License

MIT, see LICENSE.

Copyright © 2023 Koni Marti

[mailing list]: https://lists.sr.ht/~konimarti/public-inbox
[Github]: http://github.com/konimarti/cical
