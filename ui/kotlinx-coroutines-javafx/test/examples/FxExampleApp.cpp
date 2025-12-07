// Transliterated from Kotlin to C++
// Original: ui/kotlinx-coroutines-javafx/test/examples/FxExampleApp.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JavaFX Application framework
// TODO: Handle suspend functions and coroutines
// TODO: Implement CoroutineScope interface
// TODO: Handle SimpleDateFormat and Date

namespace examples {

// TODO: import javafx.application.*
// TODO: import javafx.scene.*
// TODO: import javafx.scene.control.*
// TODO: import javafx.scene.layout.*
// TODO: import javafx.scene.paint.*
// TODO: import javafx.scene.shape.*
// TODO: import javafx.stage.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.javafx.*
// TODO: import java.text.*
// TODO: import java.util.*
// TODO: import kotlin.coroutines.*

void main(const std::vector<std::string>& args) {
    Application::launch<FxTestApp>(args);
}

void log(const std::string& msg) {
    // TODO: SimpleDateFormat implementation
    std::cout << "yyyyMMdd-HHmmss.sss [" << std::this_thread::get_id() << "] " << msg << std::endl;
}

class FxTestApp : public Application, public CoroutineScope {
private:
    FlowPane buttons_{};
    Pane root_{};
    Scene scene_{&root_, 600.0, 400.0};
    Random random_{};
    int animation_index_ = 0;
    Job job_{};

public:
    FxTestApp() {
        auto rect_button = new Button("Rect");
        rect_button->set_on_action([this](auto) { do_rect(); });
        buttons_.children() += rect_button;

        auto circle_button = new Button("Circle");
        circle_button->set_on_action([this](auto) { do_circle(); });
        buttons_.children() += circle_button;

        auto clear_button = new Button("Clear");
        clear_button->set_on_action([this](auto) { do_clear(); });
        buttons_.children() += clear_button;

        root_.children() += &buttons_;
    }

    void start(Stage* stage) override {
        stage->set_title("Hello world!");
        stage->set_scene(&scene_);
        stage->show();
    }

    CoroutineContext coroutine_context() const override {
        return Dispatchers::JavaFx + job_;
    }

private:
    void animation(Node* node, std::function<void()> block) {
        root_.children() += node;
        launch(block).also([&, node](auto it) {
            it.invoke_on_completion([&, node](auto) {
                root_.children() -= node;
            });
        });
    }

    void do_rect() {
        auto node = new Rectangle(20.0, 20.0);
        node->set_fill(Color::kRed);

        int index = ++animation_index_;
        double speed = 5.0;
        animation(node, [=]() {
            log("Started new 'rect' coroutine #" + std::to_string(index));
            double vx = speed;
            double vy = speed;
            int counter = 0;
            while (true) {
                await_pulse();
                node->set_x(node->x() + vx);
                node->set_y(node->y() + vy);
                auto x_range = std::make_pair(0.0, scene_.width() - node->width());
                auto y_range = std::make_pair(0.0, scene_.height() - node->height());
                if (node->x() < x_range.first || node->x() > x_range.second) {
                    node->set_x(std::clamp(node->x(), x_range.first, x_range.second));
                    vx = -vx;
                }
                if (node->y() < y_range.first || node->y() > y_range.second) {
                    node->set_y(std::clamp(node->y(), y_range.first, y_range.second));
                    vy = -vy;
                }
                if (counter++ > 100) {
                    counter = 0;
                    delay(1000); // pause a bit
                    log("Delayed #" + std::to_string(index) + " for a while, resume and turn");
                    double t = vx;
                    vx = vy;
                    vy = -t;
                }
            }
        });
    }

    void do_circle() {
        auto node = new Circle(20.0);
        node->set_fill(Color::kBlue);

        int index = ++animation_index_;
        double acceleration = 0.1;
        double max_speed = 5.0;
        animation(node, [=]() {
            log("Started new 'circle' coroutine #" + std::to_string(index));
            double sx = random_.next_double() * max_speed;
            double sy = random_.next_double() * max_speed;
            while (true) {
                await_pulse();
                double dx = root_.width() / 2 - node->translate_x();
                double dy = root_.height() / 2 - node->translate_y();
                double dn = std::sqrt(dx * dx + dy * dy);
                sx += dx / dn * acceleration;
                sy += dy / dn * acceleration;
                double sn = std::sqrt(sx * sx + sy * sy);
                double trim = std::min(sn, max_speed);
                sx = sx / sn * trim;
                sy = sy / sn * trim;
                node->set_translate_x(node->translate_x() + sx);
                node->set_translate_y(node->translate_y() + sy);
            }
        });
    }

    void do_clear() {
        job_.cancel();
        job_ = Job();
    }
};

} // namespace examples
