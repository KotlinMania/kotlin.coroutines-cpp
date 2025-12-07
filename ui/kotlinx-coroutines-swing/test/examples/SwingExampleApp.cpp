// Transliterated from Kotlin to C++
// Original: ui/kotlinx-coroutines-swing/test/examples/SwingExampleApp.kt
// TODO: Resolve imports and dependencies
// TODO: Implement Swing framework (JFrame, JProgressBar, JTextArea, JPanel)
// TODO: Handle suspend functions and coroutines
// TODO: Implement CompletableFuture integration

namespace examples {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.future.*
// TODO: import kotlinx.coroutines.swing.*
// TODO: import java.awt.*
// TODO: import java.util.concurrent.*
// TODO: import javax.swing.*

void create_and_show_gui() {
    auto frame = new JFrame("Async UI example");
    frame->set_default_close_operation(JFrame::kExitOnClose);

    auto j_progress_bar = new JProgressBar(0, 100);
    j_progress_bar->set_value(0);
    j_progress_bar->set_is_string_painted(true);

    auto j_text_area = new JTextArea(11, 10);
    j_text_area->set_margin(Insets(5, 5, 5, 5));
    j_text_area->set_is_editable(false);

    auto panel = new JPanel();

    panel->add(j_progress_bar);
    panel->add(j_text_area);

    frame->content_pane().add(panel);
    frame->pack();
    frame->set_is_visible(true);

    GlobalScope::launch(Dispatchers::Swing, [=]() {
        for (int i = 1; i <= 10; i++) {
            // 'append' method and consequent 'jProgressBar.setValue' are called
            // within Swing event dispatch thread
            j_text_area->append(
                start_long_async_operation(i).await()
            );
            j_progress_bar->set_value(i * 10);
        }
    });
}

CompletableFuture<std::string> start_long_async_operation(int v) {
    return CompletableFuture::supply_async([v]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return "Message: " + std::to_string(v) + "\n";
    });
}

void main(const std::vector<std::string>& args) {
    SwingUtilities::invoke_later(&create_and_show_gui);
}

} // namespace examples
