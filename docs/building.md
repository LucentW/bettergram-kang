# Build instructions

* [Visual Studio 2017](building-msvc.md)
* [Xcode 9](building-xcode.md)
* [GYP/CMake on GNU/Linux](building-cmake.md)

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
