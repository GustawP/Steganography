#include <SFML/Graphics.hpp>
#include <fmt/core.h>
#include <string>
#include <bitset>
#include <random>
#include <vector>
#include <iostream>
#include <filesystem>
#include <format>
#include <chrono>

// Program stworzony przez: GUSTAW PLUTA, na potrzeby projektu na studia dnia 13.06.2024 r.
// Steganografia obrazowa

// -e C:\Users\gutek\Desktop\zdjecie.bmp "Sekretna wiadomosc .bmp abc123!"
// -e C:\Users\gutek\Desktop\zdjecie.png "Wiadomosc sekretna .png abc123?"
// -e C:\Users\gutek\Desktop\zdjecie.bmp "ęąśćż"

// -d C:\Users\gutek\Desktop\zdjecie.bmp 31
// -d C:\Users\gutek\Desktop\zdjecie.png 31

// -c C:\Users\gutek\Desktop\zdjecie.bmp "Czy sie zmiesci to"
// -c C:\Users\gutek\Desktop\zdjecie.png "Czy to sie zmiesci"

// -i C:\Users\gutek\Desktop\zdjecie.bmp
// -i C:\Users\gutek\Desktop\zdjecie.png

auto generate_random_numbers_from_password(const std::string &password, int count) -> std::vector<int>;
auto embed_message_in_image(sf::Image &image, const std::string &password, const std::string &message) -> void;
auto decode_message_from_image(const sf::Image &image, const std::string &password, int message_length) -> std::string;
auto print_file_info(const std::string &file_path) -> void;
auto check_embedding_possibility(const std::string &file_path, const std::string &message) -> void;

auto main(int argc, char *argv[]) -> int {
    // Sprawdzenie, czy podano wystarczajaca liczbe argumentow
    if (argc < 2) {
        fmt::print("Brak argumentow. Uzyj -h lub --help, aby wyswietlic pomoc.\n");
        return -1;
    }

    std::string flag = argv[1];

    // Obsluga flagi -h lub --help
    if (flag == "-h" || flag == "--help") {
        fmt::print("Uzycie:\n");
        fmt::print("  -i, --info     <sciezka do pliku>                        Wyswietla informacje o pliku\n");
        fmt::print("  -e, --encrypt  <sciezka do pliku>  \"<wiadomosc>\"         Szyfruje wiadomosc w pliku\n");
        fmt::print("  -d, --decrypt  <sciezka do pliku>  <dlugosc wiadomosci>  Odszyfrowuje wiadomosc z pliku\n");
        fmt::print("  -c, --check    <sciezka do pliku>  \"<wiadomosc>\"         Sprawdza mozliwosc zapisania wiadomosci\n");
        fmt::print("  -h, --help                                               Wyswietla pomoc\n");
        return 0;
    }

    // Obsluga flagi -i lub --info
    if (flag == "-i" || flag == "--info") {
        if (argc != 3) {
            fmt::print("Nieprawidlowe uzycie flagi -i. Uzyj -h lub --help, aby uzyskac pomoc.\n");
            return -1;
        }
        std::string file_path = argv[2];
        print_file_info(file_path);
        return 0;
    }

    // Obsluga flagi -e lub --encrypt
    if (flag == "-e" || flag == "--encrypt") {
        if (argc != 4) {
            fmt::print("Nieprawidlowe uzycie flagi -e. Uzyj -h lub --help, aby uzyskac pomoc.\n");
            return -1;
        }
        std::string file_path = argv[2];
        std::string message = argv[3];

        sf::Image image;
        if (!image.loadFromFile(file_path)) {
            fmt::print("Nie udalo sie zaladowac obrazu\n");
            return -1;
        }

        std::string password;
        fmt::print("Wprowadz haslo: ");
        std::getline(std::cin, password);

        embed_message_in_image(image, password, message);

        if (!image.saveToFile(file_path)) {
            fmt::print("Nie udalo sie zapisac obrazu\n");
            return -1;
        }
        fmt::print("Wiadomosc zostala zaszyfrowana w pliku {}\n", file_path);
        return 0;
    }

    // Obsluga flagi -d lub --decrypt
    if (flag == "-d" || flag == "--decrypt") {
        if (argc != 4) {
            fmt::print("Nieprawidlowe uzycie flagi -d. Uzyj -h lub --help, aby uzyskac pomoc.\n");
            return -1;
        }
        std::string file_path = argv[2];
        auto message_length = std::stoi(argv[3]);

        sf::Image image;
        if (!image.loadFromFile(file_path)) {
            fmt::print("Nie udalo sie zaladowac obrazu\n");
            return -1;
        }

        std::string password;
        fmt::print("Wprowadz haslo: ");
        std::getline(std::cin, password);

        std::string decoded_message = decode_message_from_image(image, password, message_length);
        fmt::print("Odszyfrowana wiadomosc: {}\n", decoded_message);
        return 0;
    }

    // Obsluga flagi -c lub --check
    if (flag == "-c" || flag == "--check") {
        if (argc != 4) {
            fmt::print("Nieprawidlowe uzycie flagi -c. Uzyj -h lub --help, aby uzyskac pomoc.\n");
            return -1;
        }
        std::string file_path = argv[2];
        std::string message = argv[3];
        check_embedding_possibility(file_path, message);
        return 0;
    }

    // Nieznana flaga
    fmt::print("Nieznana flaga. Uzyj -h lub --help, aby uzyskac pomoc.\n");
    return -1;
}

auto generate_random_numbers_from_password(const std::string &password, int count) -> std::vector<int> {
    std::hash<std::string> hash_fn;
    size_t seed = hash_fn(password);
    std::mt19937 generator(seed);
    std::uniform_int_distribution<> distribution(0, std::numeric_limits<int>::max());
    std::vector<int> random_numbers(count);
    for (auto i = 0; i < count; ++i) {
        random_numbers[i] = distribution(generator);
    }
    return random_numbers;
}

auto embed_message_in_image(sf::Image &image, const std::string &password, const std::string &message) -> void {
    auto width = image.getSize().x;
    auto height = image.getSize().y;
    auto pixel_count = width * height;

    // Konwersja wiadomosci na postac binarna
    std::vector<bool> binary_message;
    for (char c: message) {
        std::bitset<8> bits(c);
        for (int i = 7; i >= 0; --i) {
            binary_message.push_back(bits[i]);
        }
    }

    // Generowanie losowych pozycji do osadzenia wiadomosci
    auto random_numbers = generate_random_numbers_from_password(password, binary_message.size());

    // Osadzanie bitow wiadomosci w LSB kanalu czerwonego
    for (size_t i = 0; i < binary_message.size(); ++i) {
        auto pos = random_numbers[i] % pixel_count;
        auto x = pos % width;
        auto y = pos / width;
        sf::Color color = image.getPixel(x, y);
        color.r = (color.r & 0xFE) | binary_message[i]; // Zastapienie LSB bitem wiadomosci
        image.setPixel(x, y, color);
    }
}

auto decode_message_from_image(const sf::Image &image, const std::string &password, int message_length) -> std::string {
    auto width = image.getSize().x;
    auto height = image.getSize().y;
    auto pixel_count = width * height;

    // Obliczanie liczby bitow w wiadomosci
    auto num_bits = message_length * 8;

    // Generowanie losowych pozycji do odczytania wiadomosci
    auto random_numbers = generate_random_numbers_from_password(password, num_bits);

    // Wyciaganie binarnej wiadomosci z obrazu
    std::vector<bool> binary_message(num_bits);
    for (auto i = 0; i < num_bits; ++i) {
        auto pos = random_numbers[i] % pixel_count;
        auto x = pos % width;
        auto y = pos / width;
        sf::Color color = image.getPixel(x, y);
        binary_message[i] = color.r & 1; // Wyciaganie LSB
    }

    // Konwersja binarnej wiadomosci na tekst
    std::string message;
    for (auto i = 0; i < num_bits; i += 8) {
        std::bitset<8> bits;
        for (auto j = 0; j < 8; ++j) {
            bits[7 - j] = binary_message[i + j];
        }
        message += static_cast<char>(bits.to_ulong());
    }
    return message;
}

auto print_file_info(const std::string &file_path) -> void {
    // Sprawdzenie, czy plik istnieje
    if (!std::filesystem::exists(file_path)) {
        fmt::print("Plik nie istnieje.\n");
        return;
    }

    std::filesystem::path path(file_path);

    // Wyswietlanie podstawowych informacji o pliku
    fmt::print("Informacje o pliku:\n");
    fmt::print("Sciezka: {}\n", path.string());
    fmt::print("Rozmiar: {} bajtow\n", std::filesystem::file_size(file_path));
    std::filesystem::file_time_type f_time = std::filesystem::last_write_time(file_path);
    std::cout << std::format("File write time is {}\n", f_time);

    // Sprawdzenie, czy plik jest obslugiwanym formatem (dla przykladu .png i .jpg)
    auto extension = path.extension().string();
    if (extension == ".png" || extension == ".jpg") {
        fmt::print("Obslugiwany format pliku: {}\n", extension);

        // Dodatkowe informacje dla plikow graficznych
        sf::Image image;
        if (image.loadFromFile(file_path)) {
            fmt::print("Wymiary obrazu: {}x{}\n", image.getSize().x, image.getSize().y);
        } else {
            fmt::print("Nie udalo sie zaladowac obrazu.\n");
        }
    } else {
        fmt::print("Nieobslugiwany format pliku: {}\n", extension);
    }
}

auto check_embedding_possibility(const std::string &file_path, const std::string &message) -> void {
    // Zaladowanie obrazu
    sf::Image image;
    if (!image.loadFromFile(file_path)) {
        fmt::print("Nie udalo sie zaladowac obrazu\n");
        return;
    }

    auto width = image.getSize().x;
    auto height = image.getSize().y;
    auto pixel_count = width * height;

    // Konwersja wiadomosci na postac binarna
    std::vector<bool> binary_message;
    for (auto c : message) {
        std::bitset<8> bits(c);
        for (auto i = 7; i >= 0; --i) {
            binary_message.push_back(bits[i]);
        }
    }

    // Sprawdzenie, czy wiadomosc zmiesci sie w obrazie
    if (binary_message.size() > pixel_count) {
        fmt::print("Wiadomosc jest zbyt dluga, aby zapisac ja w podanym obrazie.\n");
    } else {
        fmt::print("Wiadomosc moze zostac zapisana w podanym obrazie.\n");
    }
}
