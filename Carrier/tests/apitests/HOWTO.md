# HOW-TO-USE
## Usages

The **elatests** must be run with configuration file of which you can specifiy it with option **-c**:

```shell
$./elatests -c YOUR-CONFIG-PATH/elatests.conf
```

 or run without options:

```shell
$ ./elatests
```

 In this case, the upper command would be internally configured with the following paths in order of search priority:

```markdown
./elatests.conf
../etc/carrier/elatests.conf
/usr/local/etc/carrier/elatests.conf
/etc/carrier/elatests.conf
```

Beware that the last two candidate paths would be neglected on Windows.

### Standalone Usages

In default, **elatests** would run with two child processes running in parallel, one is to run as a robot to backend testing, the other is the main body to verify all exported APIs.  Please use `ps` command to check all running processes of **elatests**.

The **elatests** can be specifically to run as the **robot** or the **testcase** in standalone with option **—robot** or **—cases**.

Assumed that under the distribution directory, run the command to run as test-suite app:

```shell
$./elatests --cases [-c YOUR-CONFIG-FILE]
```

Or run the command below to run as the robot to backend testing:

```shell
$./elatests --robot [-c YOUR-CONFIG-FILE]
```

Notice that the environment value **DYLD_LIBRARY_PATH** should be explicitly set to the path of dynamic libraries when running on MacOS:

```shell
$DYLD_LIBRARY_PATH=../lib ./elatests [--cases | --robot][-c YOUR-CONFIG-FILE]
```

On windows, run the following command:

```shell
$elatests.exe [--cases | --robot][-c YOUR-CONFIG-FILE]
```