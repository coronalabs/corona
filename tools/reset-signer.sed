s/^[[:space:]]*"CODE_SIGN_IDENTITY\[sdk=iphoneos\*\]" = "iPhone Developer:[[:space:]]?[[:alpha:] ]*[[:space:]]?\([[:alpha:][:digit:]]*\)"\;$/                      CODE_SIGN_IDENTITY = "iPhone Developer: sean Head (45AW7676F8)";/g
s/^[[:space:]]*CODE_SIGN_IDENTITY = "iPhone Developer:[[:space:]]?[[:alpha:] ]*[[:space:]]?\([[:alpha:][:digit:]]*\)"\;$/                      CODE_SIGN_IDENTITY = "iPhone Developer: sean Head (45AW7676F8)";/g
s/^[[:space:]]*"PROVISIONING_PROFILE\[sdk=iphoneos\*\]" = "[0123456789ABCDEF-]*"\;$/        "PROVISIONING_PROFILE[sdk=iphoneos*]" = "93041999-1DC1-40B4-A756-0D0E9C3876B6";/g
s/^[[:space:]]*PROVISIONING_PROFILE = "[0123456789ABCDEF-]*"\;$/       PROVISIONING_PROFILE = "93041999-1DC1-40B4-A756-0D0E9C3876B6";/g
