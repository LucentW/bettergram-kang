# [Bettergram][bettergram] – Official Messenger

This is the complete source code and the build instructions for the official desktop client for the [Bettergram][bettergram] messenger, based on the [Telegram API][telegram_api] and the [MTProto][telegram_proto] secure protocol.

[![Preview of Bettergram][preview_image]][preview_image_url]

The source code is published under GPLv3 with OpenSSL exception, the license is available [here][license].

## Supported systems

* Windows XP - Windows 10 (**not** RT)
* Mac OS X 10.8 - Mac OS X 10.11
* Ubuntu 12.04 - Ubuntu 18.04
* Fedora 22 - Fedora 28

## Third-party

* Qt 5.3.2 and 5.6.2, slightly patched ([LGPL](http://doc.qt.io/qt-5/lgpl.html))
* OpenSSL 1.0.1g ([OpenSSL License](https://www.openssl.org/source/license.html))
* zlib 1.2.8 ([zlib License](http://www.zlib.net/zlib_license.html))
* libexif 0.6.20 ([LGPL](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html))
* LZMA SDK 9.20 ([public domain](http://www.7-zip.org/sdk.html))
* liblzma ([public domain](http://tukaani.org/xz/))
* Google Breakpad ([License](https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE))
* Google Crashpad ([Apache License 2.0](https://chromium.googlesource.com/crashpad/crashpad/+/master/LICENSE))
* GYP ([BSD License](https://github.com/bnoordhuis/gyp/blob/master/LICENSE))
* Ninja ([Apache License 2.0](https://github.com/ninja-build/ninja/blob/master/COPYING))
* OpenAL Soft ([LGPL](http://kcat.strangesoft.net/openal.html))
* Opus codec ([BSD License](http://www.opus-codec.org/license/))
* FFmpeg ([LGPL](https://www.ffmpeg.org/legal.html))
* Guideline Support Library ([MIT License](https://github.com/Microsoft/GSL/blob/master/LICENSE))
* Mapbox Variant ([BSD License](https://github.com/mapbox/variant/blob/master/LICENSE))
* Range-v3 ([Boost License](https://github.com/ericniebler/range-v3/blob/master/LICENSE.txt))
* Open Sans font ([Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html))
* Emoji alpha codes ([MIT License](https://github.com/emojione/emojione/blob/master/extras/alpha-codes/LICENSE.md))
* Catch test framework ([Boost License](https://github.com/philsquared/Catch/blob/master/LICENSE.txt))

## Build instructions

* [Visual Studio 2017][msvc]
* [Xcode 9][xcode]
* [GYP/CMake on GNU/Linux][cmake]

## Customize application

Before publish a custom version of Telegram Desktop application you need to get new `api_id` and `api_hash` values.
To do that you need to follow instructions at the page: [Obtaining api_id](https://core.telegram.org/api/obtaining_api_id).
See also the thread: [GitHub: "internal server error" on login](https://github.com/telegramdesktop/tdesktop/issues/4717).

So, after obtaining new values of `api_id` and `api_hash` you need to create a new directory next to your `tdesktop` folder with the name: `TelegramPrivate`.
And place a new file `custom_api_id.h` there with the content:

```
static const int32 ApiId = [api_id];
static const char *ApiHash = "[api_hash]";
```

And please check that we set `CUSTOM_API_ID` define at the `Telegram.gyp` file.

Also please note that we do not need to place the `custom_api_id.h` to Git.
It is because we should keep `api_id` and `api_hash` values private.

### Autoupdates

If you want to support autoupdates feature you should do the following things:

1. Change autoupdate url
2. Generate your own `UpdatesPublicKey` and `UpdatesPublicAlphaKey` RSA keys
3. Build `Packer` project
4. Make update packages with `Packer` utility
5. Setup your own server for maintaining update packages

#### Changing autoupdate url

The current autoupdate url is `https://updates.bettergram.io`. It is hard coded at the `readAutoupdatePrefixRaw()` method from `localstorage.cpp` file. In order to temporary change this value you can create `<working-directory>/tdata/prefix` file with new autoupdate url value. So, if this file exists and does not empty the application uses the file content as an autoupdate url prefix.

#### Generating UpdatesPublicKey and UpdatesPublicAlphaKey

In order to be able to use your own updates you should replace `UpdatesPublicKey` and `UpdatesPublicAlphaKey` variables at the `config.h` and `packer.cpp` files. To do that you need to generate two RSA keys by using the following commands:

```
$ openssl genrsa -passout stdin -out bettergram-updates-key-private.pem 1024
$ openssl rsa -in bettergram-updates-key-private.pem -RSAPublicKey_out -out bettergram-updates-key-public.pem

$ openssl genrsa -passout stdin -out bettergram-updates-alpha-key-private.pem 1024
$ openssl rsa -in bettergram-updates-alpha-key-private.pem -RSAPublicKey_out -out bettergram-updates-alpha-key-public.pem
```

Please do not forget to assign strong passwords for both keys (you should type them right after you typed commands to generate private keys). After you will have generated them you should replace `UpdatesPublicKey` and `UpdatesPublicAlphaKey` variables at the `config.h` and `packer.cpp` files with contents of the `*-public.pem` keys and store private keys in safe place and do **NOT** publish it or store in git.

#### Making update packages

There is `Packer` project in the Bettergram solution. In order to make update packages you should do the following steps:

1. Add `packer_private.h` file to the `TelegramPrivate` directory. This file should contain `PrivateKey` and `PrivateAlphaKey` variables:

   ```
   const char *PrivateKey = "\
   -----BEGIN RSA PRIVATE KEY-----\n\
   ...
   -----END RSA PRIVATE KEY-----\
   ";

   const char *PrivateAlphaKey = "\
   -----BEGIN RSA PRIVATE KEY-----\n\
   ...
   -----END RSA PRIVATE KEY-----\
   ";
   ```

2. Add `beta_private.h` file to the `TelegramPrivate` directory. This file should contain `BetaPrivateKey` variable:

   ```
   const char *BetaPrivateKey = "\
   -----BEGIN RSA PRIVATE KEY-----\n\
   ...
   -----END RSA PRIVATE KEY-----\
   ";
   ```

   I assume this value may be equal to `PrivateAlphaKey` variable.

3. Build the `Packer` project for each platform (Linux, macOS and Windows)
4. Run `Packer` utility:

   ```
   $ ./Packer -path Bettergram -path Updater -version 1003014
   ```

   You should use actual Bettergram version number instead of `1003014`. The `Packer` should generate one file that will contain all files from `-path` arguments, something like this: `tlinuxupd1003014`

[//]: # (LINKS)
[bettergram]: https://bettergram.io
[telegram_api]: https://core.telegram.org
[telegram_proto]: https://core.telegram.org/mtproto
[license]: LICENSE
[msvc]: docs/building-msvc.md
[xcode]: docs/building-xcode.md
[xcode_old]: docs/building-xcode-old.md
[cmake]: docs/building-cmake.md
[preview_image]: https://github.com/telegramdesktop/tdesktop/blob/dev/docs/assets/preview.png "Preview of Bettergram"
[preview_image_url]: https://raw.githubusercontent.com/telegramdesktop/tdesktop/dev/docs/assets/preview.png
