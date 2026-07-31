# ByteBuilders Compiler GUI

This folder has the Java Swing GUI for the compiler project. It is a simple window where you can open a `.mc` file, write code, and run the compiler from inside the app.

## Folder Structure

- `src/Main.java` starts the GUI.
- `src/CompilerGUI.java` makes the main window.
- `src/CompilerRunner.java` saves the code in a temp file and runs the `compiler` binary.
- `src/OutputConsole.java` shows the compiler output.

## How It Works

1. You open a `.mc` file or type code in the editor.
2. When you press **Run**, the GUI saves the code in a temporary file.
3. Then it runs the built `compiler` binary on that file.
4. The output comes back in the console area below.
5. If there is an error line, it is shown in red.

## How to Run on Ubuntu

From the project root, run these commands:

```bash
make
javac gui/src/*.java
java -cp gui/src Main
```

If the GUI cannot find the compiler binary, use this:

```bash
java -Dcompiler.path="$PWD/compiler" -cp gui/src Main
```

## Notes

- The GUI expects the compiler binary to be named `compiler`.
- Keep the `compiler` file in the project root.
- You only need the `.java` files in GitHub. The `.class` files are generated after compile time.