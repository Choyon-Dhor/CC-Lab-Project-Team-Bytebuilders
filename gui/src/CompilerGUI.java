import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.io.File;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;

import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JToolBar;
import javax.swing.SwingWorker;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.filechooser.FileNameExtensionFilter;

public class CompilerGUI extends JFrame {
    private final JTextArea editorArea;
    private final OutputConsole outputConsole;
    private final CompilerRunner compilerRunner;
    private final JLabel statusLabel;

    private File currentFile;

    public CompilerGUI() {
        super("ByteBuilders Compiler GUI");
        compilerRunner = new CompilerRunner();

        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setMinimumSize(new Dimension(1100, 760));
        setLocationByPlatform(true);

        editorArea = new JTextArea();
        editorArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 14));
        editorArea.setBackground(new Color(0x11, 0x17, 0x1D));
        editorArea.setForeground(new Color(0xF3, 0xF4, 0xF6));
        editorArea.setCaretColor(new Color(0xF3, 0xF4, 0xF6));
        editorArea.setSelectionColor(new Color(0x22, 0x3A, 0x5E));
        editorArea.setTabSize(4);
        editorArea.setBorder(BorderFactory.createEmptyBorder(12, 12, 12, 12));
        editorArea.setText(defaultSample());

        editorArea.getDocument().addDocumentListener(new DocumentListener() {
            @Override
            public void insertUpdate(DocumentEvent e) {
                updateStatus();
            }

            @Override
            public void removeUpdate(DocumentEvent e) {
                updateStatus();
            }

            @Override
            public void changedUpdate(DocumentEvent e) {
                updateStatus();
            }
        });

        outputConsole = new OutputConsole();
        statusLabel = new JLabel();
        statusLabel.setBorder(BorderFactory.createEmptyBorder(4, 8, 4, 8));

        setJMenuBar(createMenuBar());
        add(createToolbar(), BorderLayout.NORTH);
        add(createMainContent(), BorderLayout.CENTER);
        add(createStatusBar(), BorderLayout.SOUTH);

        updateStatus();
        pack();
    }

    private JMenuBar createMenuBar() {
        JMenuBar menuBar = new JMenuBar();

        JMenu fileMenu = new JMenu("File");
        fileMenu.add(new JMenuItem(new AbstractAction("Open...") {
            @Override
            public void actionPerformed(ActionEvent event) {
                openFile();
            }
        }));
        fileMenu.add(new JMenuItem(new AbstractAction("Run") {
            @Override
            public void actionPerformed(ActionEvent event) {
                runCompiler();
            }
        }));
        fileMenu.addSeparator();
        fileMenu.add(new JMenuItem(new AbstractAction("Exit") {
            @Override
            public void actionPerformed(ActionEvent event) {
                dispose();
            }
        }));

        JMenu buildMenu = new JMenu("Build");
        buildMenu.add(new JMenuItem(new AbstractAction("Run Compiler") {
            @Override
            public void actionPerformed(ActionEvent event) {
                runCompiler();
            }
        }));

        menuBar.add(fileMenu);
        menuBar.add(buildMenu);
        return menuBar;
    }

    private JToolBar createToolbar() {
        JToolBar toolBar = new JToolBar();
        toolBar.setFloatable(false);
        toolBar.setBorder(BorderFactory.createEmptyBorder(6, 8, 6, 8));

        JButton openButton = new JButton("Open");
        openButton.addActionListener(event -> openFile());

        JButton runButton = new JButton("Run");
        runButton.addActionListener(event -> runCompiler());

        toolBar.add(openButton);
        toolBar.add(Box.createHorizontalStrut(8));
        toolBar.add(runButton);
        toolBar.add(Box.createHorizontalGlue());

        JLabel hint = new JLabel("Open or paste a .mc file, then click Run.");
        toolBar.add(hint);

        return toolBar;
    }

    private JSplitPane createMainContent() {
        JScrollPane editorScrollPane = new JScrollPane(editorArea);
        editorScrollPane.setBorder(BorderFactory.createTitledBorder("Code Editor"));

        JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, editorScrollPane, outputConsole);
        splitPane.setResizeWeight(0.62);
        splitPane.setDividerLocation(470);
        splitPane.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
        return splitPane;
    }

    private JPanel createStatusBar() {
        JPanel statusBar = new JPanel(new BorderLayout());
        statusBar.setBorder(BorderFactory.createMatteBorder(1, 0, 0, 0, new Color(0x2D, 0x36, 0x45)));
        statusBar.add(statusLabel, BorderLayout.WEST);
        return statusBar;
    }

    private void openFile() {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileFilter(new FileNameExtensionFilter("Mini language source (*.mc)", "mc"));
        if (currentFile != null) {
            chooser.setSelectedFile(currentFile);
        }

        int choice = chooser.showOpenDialog(this);
        if (choice != JFileChooser.APPROVE_OPTION) {
            return;
        }

        File selectedFile = chooser.getSelectedFile();
        try {
            String content = Files.readString(selectedFile.toPath(), StandardCharsets.UTF_8);
            editorArea.setText(content);
            currentFile = selectedFile;
            outputConsole.appendInfo("Loaded " + selectedFile.getName());
            updateStatus();
        } catch (Exception exception) {
            showError("Could not open file: " + exception.getMessage());
        }
    }

    private void runCompiler() {
        final String sourceCode = editorArea.getText();
        outputConsole.clear();
        outputConsole.appendInfo("Running compiler...");
        setBusy(true);

        SwingWorker<CompilerRunner.Result, Void> worker = new SwingWorker<>() {
            @Override
            protected CompilerRunner.Result doInBackground() throws Exception {
                return compilerRunner.run(sourceCode);
            }

            @Override
            protected void done() {
                setBusy(false);
                try {
                    CompilerRunner.Result result = get();
                    outputConsole.appendCompilerOutput(result.output, result.success());
                    if (result.success()) {
                        outputConsole.appendSuccess("Compiler finished successfully.");
                    } else {
                        outputConsole.appendError("Compiler exited with code " + result.exitCode + ".");
                    }
                } catch (Exception exception) {
                    outputConsole.clear();
                    outputConsole.appendError(exception.getMessage());
                }
            }
        };

        worker.execute();
    }

    private void setBusy(boolean busy) {
        editorArea.setEnabled(!busy);
        statusLabel.setText(busy ? "Running compiler..." : buildStatusText());
    }

    private void updateStatus() {
        if (editorArea != null && editorArea.isEnabled()) {
            statusLabel.setText(buildStatusText());
        }
    }

    private String buildStatusText() {
        String compilerPath = compilerRunner.getCompilerPath();
        String compilerState = compilerPath != null ? "Compiler: " + compilerPath : "Compiler: auto-detecting ./compiler";
        String fileState = currentFile != null ? "File: " + currentFile.getName() : "File: untitled.mc";
        return fileState + " | " + compilerState;
    }

    private void showError(String message) {
        JOptionPane.showMessageDialog(this, message, "ByteBuilders Compiler GUI", JOptionPane.ERROR_MESSAGE);
    }

    private String defaultSample() {
        return String.join(System.lineSeparator(),
            "int x;",
            "int y;",
            "x = 10;",
            "y = x + 5;",
            "print y;",
            "");
    }
}