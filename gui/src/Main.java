import javax.swing.SwingUtilities;

public class Main {
    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            CompilerGUI gui = new CompilerGUI();
            gui.setVisible(true);
        });
    }
}