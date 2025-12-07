// Transliterated from Kotlin to C++
// Original: ui/kotlinx-coroutines-javafx/test/examples/FxAsFlow.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JavaFX Application framework
// TODO: Handle suspend functions and coroutines
// TODO: Implement CoroutineScope interface

namespace examples {

// TODO: import javafx.application.Application
// TODO: import javafx.scene.Scene
// TODO: import javafx.scene.control.*
// TODO: import javafx.scene.layout.GridPane
// TODO: import javafx.stage.Stage
// TODO: import javafx.beans.property.SimpleStringProperty
// TODO: import javafx.event.EventHandler
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.flow.*
// TODO: import kotlinx.coroutines.javafx.*
// TODO: import kotlin.coroutines.CoroutineContext

void main(const std::vector<std::string>& args) {
    Application::launch<FxAsFlowApp>(args);
}

// Adapted from
// https://github.com/ReactiveX/RxJavaFX/blob/a78ca7d15f7d82d201df8fafb6eba732ec17e327/src/test/java/io/reactivex/rxjavafx/RxJavaFXTest.java
class FxAsFlowApp : public Application, public CoroutineScope {
private:
    Job job_{};
    Button increment_button_{"Increment"};
    Label increment_label_{""};
    TextField text_input_{};
    Label flipped_text_label_{};
    Spinner<int> spinner_{};
    Label spinner_changes_label_{};

public:
    CoroutineContext coroutine_context() const override {
        return JavaFx + job_;
    }

    void start(Stage* primary_stage) override {
        auto grid_pane = new GridPane();
        grid_pane->set_hgap(10.0);
        grid_pane->set_vgap(10.0);
        grid_pane->add(&increment_button_, 0, 0);
        grid_pane->add(&increment_label_, 1, 0);
        grid_pane->add(&text_input_, 0, 1);
        grid_pane->add(&flipped_text_label_, 1, 1);
        grid_pane->add(&spinner_, 0, 2);
        grid_pane->add(&spinner_changes_label_, 1, 2);

        auto scene = new Scene(grid_pane);
        primary_stage->set_width(275.0);
        primary_stage->set_scene(scene);
        primary_stage->show();
    }

    void stop() override {
        Application::stop();
        job_.cancel();
        job_ = Job();
    }

    FxAsFlowApp() {
        // Initializing the "Increment" button
        auto string_property = SimpleStringProperty();
        int i = 0;
        increment_button_.set_on_action(EventHandler([&, string_property](auto event) {
            i += 1;
            string_property.set(std::to_string(i));
        }));
        launch([&, string_property]() {
            string_property.as_flow().collect([&](auto it) {
                if (it != nullptr) {
                    string_property.set(it);
                }
            });
        });
        increment_label_.text_property().bind(string_property);

        // Initializing the reversed text field
        auto string_property2 = SimpleStringProperty();
        launch([&, string_property2]() {
            text_input_.text_property().as_flow().collect([&](auto it) {
                if (it != nullptr) {
                    std::string reversed(it.rbegin(), it.rend());
                    string_property2.set(reversed);
                }
            });
        });
        flipped_text_label_.text_property().bind(string_property2);

        // Initializing the spinner
        spinner_.set_value_factory(new SpinnerValueFactory::IntegerSpinnerValueFactory(0, 100));
        spinner_.set_is_editable(true);
        auto string_property3 = SimpleStringProperty();
        launch([&, string_property3]() {
            spinner_.value_property().as_flow().collect([&](auto it) {
                if (it != nullptr) {
                    string_property3.set("NEW: " + std::to_string(*it));
                }
            });
        });
        spinner_changes_label_.text_property().bind(string_property3);
    }
};

} // namespace examples
