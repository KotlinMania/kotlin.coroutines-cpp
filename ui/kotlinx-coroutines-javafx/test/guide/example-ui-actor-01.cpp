// This file was automatically generated from coroutines-guide-ui.md by Knit tool. Do not edit.
// Transliterated from Kotlin to C++
// Original: ui/kotlinx-coroutines-javafx/test/guide/example-ui-actor-01.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JavaFX Application framework
// TODO: Handle suspend functions and coroutines

namespace kotlinx {
namespace coroutines {
namespace javafx {
namespace guide {
namespace example_ui_actor_01 {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.javafx.JavaFx as Main
// TODO: import javafx.application.Application
// TODO: import javafx.event.EventHandler
// TODO: import javafx.geometry.*
// TODO: import javafx.scene.*
// TODO: import javafx.scene.input.MouseEvent
// TODO: import javafx.scene.layout.StackPane
// TODO: import javafx.scene.paint.Color
// TODO: import javafx.scene.shape.Circle
// TODO: import javafx.scene.text.Text
// TODO: import javafx.stage.Stage

void main(const std::vector<std::string>& args) {
    Application::launch<ExampleApp>(args);
}

class ExampleApp : public Application {
private:
    Text hello_{"Hello World!"};
    Circle fab_{20.0, Color::value_of("#FF4081")};
    StackPane root_{};
    Scene scene_{&root_, 240.0, 380.0};

public:
    ExampleApp() {
        hello_.set_fill(Color::value_of("#C0C0C0"));

        root_.children() += &hello_;
        root_.children() += &fab_;
        StackPane::set_alignment(&hello_, Pos::kCenter);
        StackPane::set_alignment(&fab_, Pos::kBottomRight);
        StackPane::set_margin(&fab_, Insets(15.0));

        scene_.set_fill(Color::value_of("#303030"));
    }

    void start(Stage* stage) override {
        stage->set_title("Example");
        stage->set_scene(&scene_);
        stage->show();
        setup(&hello_, &fab_);
    }
};

void setup(Text* hello, Circle* fab) {
    on_click(fab, [hello](MouseEvent* event) { // start coroutine when the circle is clicked
        for (int i = 10; i >= 1; i--) { // countdown from 10 to 1
            hello->set_text("Countdown " + std::to_string(i) + " ..."); // update text
            delay(500); // wait half a second
        }
        hello->set_text("Done!");
    });
}

void on_click(Node* node, std::function<void(MouseEvent*)> action) {
    node->set_on_mouse_clicked(EventHandler([action](MouseEvent* event) {
        GlobalScope::launch(Dispatchers::Main, [action, event]() {
            action(event);
        });
    }));
}

} // namespace example_ui_actor_01
} // namespace guide
} // namespace javafx
} // namespace coroutines
} // namespace kotlinx
