#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <vector>

constexpr int SCREEN_W = 960;
constexpr int SCREEN_H = 540;
constexpr int MAP_W = 16;
constexpr int MAP_H = 16;
constexpr float PI = 3.14159265358979323846f;

constexpr std::array<const char*, MAP_H> world = {
    "1111111111111111",
    "1000000000000001",
    "1011100000110001",
    "1000100000010001",
    "1000100110010001",
    "1000000100000001",
    "1000000100111001",
    "1001100000000001",
    "1000100000000001",
    "1000100001100001",
    "1000000000100001",
    "1000000000100001",
    "1000110000000001",
    "1000000000000001",
    "1000000000000001",
    "1111111111111111"
};

bool wallAt(float x, float y) {
    const int mx = static_cast<int>(std::floor(x));
    const int my = static_cast<int>(std::floor(y));
    return mx < 0 || my < 0 || mx >= MAP_W || my >= MAP_H || world[my][mx] == '1';
}

int main() {
    sf::RenderWindow window(sf::VideoMode({SCREEN_W, SCREEN_H}), "DoomCPP");
    window.setFramerateLimit(60);

    sf::Image frame({SCREEN_W, SCREEN_H}, sf::Color::Black);
    sf::Texture texture(frame);
    sf::Sprite sprite(texture);
    sf::Clock clock;

    float px = 3.5f, py = 3.5f, angle = 0.35f;
    bool showVisualizer = true;
    constexpr float fov = PI / 3.f;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
                if (key->code == sf::Keyboard::Key::Tab) showVisualizer = !showVisualizer;
            }
        }

        const float dt = std::min(clock.restart().asSeconds(), 0.05f);
        const float turn = 2.1f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) angle -= turn;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) angle += turn;

        const float dx = std::cos(angle), dy = std::sin(angle);
        const float sx = -dy, sy = dx;
        float vx = 0.f, vy = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) { vx += dx; vy += dy; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) { vx -= dx; vy -= dy; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) { vx -= sx; vy -= sy; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) { vx += sx; vy += sy; }
        const float mag = std::hypot(vx, vy);
        if (mag > 0.f) {
            const float step = 3.f * dt / mag;
            const float nx = px + vx * step, ny = py + vy * step;
            if (!wallAt(nx + (vx > 0 ? .18f : -.18f), py)) px = nx;
            if (!wallAt(px, ny + (vy > 0 ? .18f : -.18f))) py = ny;
        }

        for (int x = 0; x < SCREEN_W; ++x) {
            const float cameraX = 2.f * x / SCREEN_W - 1.f;
            const float rayX = dx + sx * std::tan(fov / 2.f) * cameraX;
            const float rayY = dy + sy * std::tan(fov / 2.f) * cameraX;
            int mapX = static_cast<int>(px), mapY = static_cast<int>(py);
            const float deltaX = rayX == 0.f ? 1e30f : std::abs(1.f / rayX);
            const float deltaY = rayY == 0.f ? 1e30f : std::abs(1.f / rayY);
            const int stepX = rayX < 0.f ? -1 : 1;
            const int stepY = rayY < 0.f ? -1 : 1;
            float sideX = (rayX < 0.f ? px - mapX : mapX + 1.f - px) * deltaX;
            float sideY = (rayY < 0.f ? py - mapY : mapY + 1.f - py) * deltaY;
            int side = 0;
            for (int i = 0; i < 64; ++i) {
                if (sideX < sideY) { sideX += deltaX; mapX += stepX; side = 0; }
                else { sideY += deltaY; mapY += stepY; side = 1; }
                if (mapX < 0 || mapY < 0 || mapX >= MAP_W || mapY >= MAP_H ||
                    world[mapY][mapX] == '1') break;
            }
            const float distance = std::max(0.05f, side == 0 ? sideX - deltaX : sideY - deltaY);
            const int height = std::min(SCREEN_H * 4, static_cast<int>(SCREEN_H / distance));
            const int top = std::max(0, SCREEN_H / 2 - height / 2);
            const int bottom = std::min(SCREEN_H, SCREEN_H / 2 + height / 2);
            const float shade = std::clamp(1.f / (1.f + distance * .12f), .22f, 1.f);
            const sf::Color wallColor(
                static_cast<std::uint8_t>(185 * shade * (side ? .72f : 1.f)),
                static_cast<std::uint8_t>(74 * shade * (side ? .72f : 1.f)),
                static_cast<std::uint8_t>(62 * shade * (side ? .72f : 1.f))
            );
            for (int y = 0; y < SCREEN_H; ++y) {
                if (y < top) frame.setPixel({static_cast<unsigned>(x), static_cast<unsigned>(y)}, sf::Color(47, 50, 65));
                else if (y < bottom) frame.setPixel({static_cast<unsigned>(x), static_cast<unsigned>(y)}, wallColor);
                else frame.setPixel({static_cast<unsigned>(x), static_cast<unsigned>(y)}, sf::Color(49, 43, 40));
            }
        }
        texture.update(frame);
        window.clear();
        window.draw(sprite);

        if (showVisualizer) {
            constexpr float cell = 19.f;
            constexpr float ox = 18.f, oy = 18.f;
            const sf::Vector2f origin{ox, oy};
            sf::RectangleShape panel({MAP_W * cell + 12.f, MAP_H * cell + 12.f});
            panel.setPosition({ox - 6.f, oy - 6.f});
            panel.setFillColor(sf::Color(12, 17, 24, 240));
            panel.setOutlineThickness(2.f);
            panel.setOutlineColor(sf::Color(225, 225, 225));
            window.draw(panel);

            for (int my = 0; my < MAP_H; ++my) {
                for (int mx = 0; mx < MAP_W; ++mx) {
                    sf::RectangleShape tile({cell - 1.f, cell - 1.f});
                    tile.setPosition({ox + mx * cell, oy + my * cell});
                    tile.setFillColor(world[my][mx] == '1' ? sf::Color(170, 65, 59) : sf::Color(36, 44, 54));
                    window.draw(tile);
                }
            }

            sf::VertexArray rays(sf::PrimitiveType::Lines);
            for (int r = 0; r <= 40; ++r) {
                const float cameraX = 2.f * r / 40.f - 1.f;
                const float rayX = dx + sx * std::tan(fov / 2.f) * cameraX;
                const float rayY = dy + sy * std::tan(fov / 2.f) * cameraX;
                int mx = static_cast<int>(px), my = static_cast<int>(py);
                const float deltaX = rayX == 0.f ? 1e30f : std::abs(1.f / rayX);
                const float deltaY = rayY == 0.f ? 1e30f : std::abs(1.f / rayY);
                const int stepX = rayX < 0.f ? -1 : 1;
                const int stepY = rayY < 0.f ? -1 : 1;
                float sideX = (rayX < 0.f ? px - mx : mx + 1.f - px) * deltaX;
                float sideY = (rayY < 0.f ? py - my : my + 1.f - py) * deltaY;
                int side = 0;
                for (int i = 0; i < 64; ++i) {
                    if (sideX < sideY) { sideX += deltaX; mx += stepX; side = 0; }
                    else { sideY += deltaY; my += stepY; side = 1; }
                    if (mx < 0 || my < 0 || mx >= MAP_W || my >= MAP_H || world[my][mx] == '1') break;
                }
                const float distance = side == 0 ? sideX - deltaX : sideY - deltaY;
                const sf::Vector2f start{ox + px * cell, oy + py * cell};
                const sf::Vector2f end{start.x + rayX * distance * cell, start.y + rayY * distance * cell};
                rays.append(sf::Vertex{start, sf::Color(255, 214, 72, 125)});
                rays.append(sf::Vertex{end, sf::Color(255, 214, 72, 180)});
            }
            window.draw(rays);
            sf::CircleShape player(5.f);
            player.setOrigin({5.f, 5.f});
            player.setPosition({ox + px * cell, oy + py * cell});
            player.setFillColor(sf::Color(42, 218, 137));
            player.setOutlineThickness(1.5f);
            player.setOutlineColor(sf::Color::White);
            window.draw(player);
            sf::VertexArray direction(sf::PrimitiveType::Lines);
            direction.append(sf::Vertex{player.getPosition(), sf::Color::White});
            direction.append(sf::Vertex{sf::Vector2f{player.getPosition().x + dx * 17.f, player.getPosition().y + dy * 17.f}, sf::Color::White});
            window.draw(direction);
        }
        window.display();
    }
}
