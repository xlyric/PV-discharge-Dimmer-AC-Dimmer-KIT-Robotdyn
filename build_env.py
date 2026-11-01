Import("env")

# expose le nom de l'environnement platformio au firmware (affiché dans l'interface web)
env.Append(CPPDEFINES=[("BUILD_ENV", env.StringifyMacro(env["PIOENV"]))])
