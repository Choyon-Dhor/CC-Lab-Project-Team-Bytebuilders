import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

public class CompilerRunner {
    public static class Result {
        public final int exitCode;
        public final String output;

        public Result(int exitCode, String output) {
            this.exitCode = exitCode;
            this.output = output;
        }

        public boolean success() {
            return exitCode == 0;
        }
    }

    private String compilerPath;

    public void setCompilerPath(String path) {
        this.compilerPath = path;
    }

    public String getCompilerPath() {
        return compilerPath;
    }

    public String locateCompiler() {
        if (compilerPath != null && new File(compilerPath).isFile()) {
            return compilerPath;
        }

        String override = System.getProperty("compiler.path");
        if (override != null && new File(override).isFile()) {
            compilerPath = override;
            return compilerPath;
        }

        File dir = new File(System.getProperty("user.dir"));
        for (int i = 0; i < 5 && dir != null; i++) {
            File candidate = new File(dir, "compiler");
            if (candidate.isFile()) {
                compilerPath = candidate.getAbsolutePath();
                return compilerPath;
            }
            dir = dir.getParentFile();
        }

        return null;
    }

    public Result run(String sourceCode) throws IOException, InterruptedException {
        String binPath = locateCompiler();
        if (binPath == null) {
            throw new FileNotFoundException(
                "Could not find the 'compiler' binary. Run 'make' at the project root first, " +
                "or launch the GUI from inside the project folder.");
        }

        Path tempFile = Files.createTempFile("gui_source_", ".mc");
        tempFile.toFile().deleteOnExit();
        Files.write(tempFile, sourceCode.getBytes(StandardCharsets.UTF_8));

        ProcessBuilder pb = new ProcessBuilder(binPath, tempFile.toAbsolutePath().toString());
        pb.redirectErrorStream(true);
        Process process = pb.start();

        StringBuilder captured = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                captured.append(line).append('\n');
            }
        }

        int exitCode = process.waitFor();
        Files.deleteIfExists(tempFile);
        return new Result(exitCode, captured.toString());
    }
}