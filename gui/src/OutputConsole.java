import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Font;
import java.util.regex.Pattern;

import javax.swing.BorderFactory;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultStyledDocument;
import javax.swing.text.Style;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyleContext;
import javax.swing.text.StyledDocument;

public class OutputConsole extends JPanel {
    private static final Pattern ERROR_PATTERN = Pattern.compile("(?i)(error|fatal|warning|failed|undefined|undeclared|mismatch|syntax|lexical|semantic)");

    private final JTextPane outputPane;
    private final StyledDocument document;
    private final Style neutralStyle;
    private final Style successStyle;
    private final Style errorStyle;

    public OutputConsole() {
        setLayout(new BorderLayout());
        setBorder(BorderFactory.createTitledBorder("Output Console"));

        outputPane = new JTextPane();
        outputPane.setEditable(false);
        outputPane.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 13));
        outputPane.setBackground(new Color(0x0F, 0x17, 0x29));
        outputPane.setForeground(new Color(0xE5, 0xE7, 0xEB));
        outputPane.setCaretColor(new Color(0xE5, 0xE7, 0xEB));

        document = new DefaultStyledDocument();
        outputPane.setDocument(document);

        StyleContext styleContext = StyleContext.getDefaultStyleContext();
        neutralStyle = styleContext.addStyle("neutral", null);
        StyleConstants.setForeground(neutralStyle, new Color(0xE5, 0xE7, 0xEB));

        successStyle = styleContext.addStyle("success", null);
        StyleConstants.setForeground(successStyle, new Color(0x4C, 0xAF, 0x50));

        errorStyle = styleContext.addStyle("error", null);
        StyleConstants.setForeground(errorStyle, new Color(0xF8, 0x5C, 0x50));

        JScrollPane scrollPane = new JScrollPane(outputPane);
        scrollPane.setBorder(BorderFactory.createEmptyBorder());
        add(scrollPane, BorderLayout.CENTER);

        appendInfo("Ready. Open a .mc file or paste code, then click Run.");
    }

    public void clear() {
        try {
            document.remove(0, document.getLength());
        } catch (BadLocationException ignored) {
        }
    }

    public void appendInfo(String text) {
        appendLine(text, neutralStyle);
    }

    public void appendSuccess(String text) {
        appendLine(text, successStyle);
    }

    public void appendError(String text) {
        appendLine(text, errorStyle);
    }

    public void appendCompilerOutput(String text, boolean success) {
        clear();
        if (text == null || text.isBlank()) {
            appendInfo("Compiler finished with no output.");
            return;
        }

        String[] lines = text.split("\\R", -1);
        for (String line : lines) {
            if (line.isBlank()) {
                appendLine("\n", neutralStyle);
            } else if (ERROR_PATTERN.matcher(line).find()) {
                appendLine(line, errorStyle);
            } else if (success) {
                appendLine(line, successStyle);
            } else {
                appendLine(line, neutralStyle);
            }
        }
    }

    private void appendLine(String text, Style style) {
        try {
            document.insertString(document.getLength(), text.endsWith("\n") ? text : text + "\n", style);
            outputPane.setCaretPosition(document.getLength());
        } catch (BadLocationException ignored) {
        }
    }
}