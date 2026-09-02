# Remaining decisions

## Autoindex

The generated directory listing currently includes the `..` entry.

This is not blocking and the autoindex feature works correctly, but we may decide to filter `..` for a cleaner listing.

## `upload_enabled`

When `POST` is allowed and `upload_enabled` is `off`, the server currently falls back to the default upload directory:

```text
www/uploads
```

We should confirm whether this is the intended behavior.

Possible choices:

- keep the current fallback to `www/uploads`;
- disable the fallback and only store uploads when `upload_enabled` is explicitly enabled.

## Virtual hosts

Virtual-host selection through `server_name` / `Host` is implemented as an extra feature.

It is not required by the subject, so we should decide whether to keep it visible in the final demonstration configuration or simply keep the implementation without making it part of the main demo.

## Final configuration files

Clarify the intended role of the two configuration files before the final README:

- `webserv.conf`: clean/default configuration;
- `test.conf`: full demonstration and test configuration.

Keeping this distinction explicit will make the project easier to present during evaluation.
