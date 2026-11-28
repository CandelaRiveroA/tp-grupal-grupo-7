#include "mock_socket.h"

#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "common/common_protocol_receiver.h"
#include "common/common_protocol_sender.h"

// Casos base: Envio y recepcion del mensaje más corto que existe (Wake, solo el byte de tipo) y
// un ServerObtained con la cadena vacía.
TEST(ProtocolMockTest, MensajeMinimoYCadenaVacia) {
    MockSocket socket;
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);

    // Wake = {5}: solo el byte de tipo (5), no tiene payload.
    sender.send_client_message(protocol::WakeMessage{});
    EXPECT_EQ(socket.sent_bytes(), (std::vector<uint8_t>{5}));
    socket.set_input({5});
    const auto command = receiver.receive_command();
    ASSERT_TRUE(command.has_value());
    EXPECT_TRUE(std::holds_alternative<protocol::WakeMessage>(*command));

    // ServerObtained con content_id vacío = {7, 0, 0}: tipo 7 + largo de la
    // cadena en 2 bytes orden de red (0x0000 = sin contenido). El socket ya
    // tenía el 5 del Wake, por eso sent_bytes acumula {5, 7, 0, 0}.
    sender.send_server_message(ServerObtained{""});
    EXPECT_EQ(socket.sent_bytes(), (std::vector<uint8_t>{5, 7, 0, 0}));
    socket.set_input({7, 0, 0});
    EXPECT_TRUE(std::get<ServerObtained>(receiver.receive_server_message()).content_id.empty());
}

// Ronda completa con la cadena más larga que el protocolo acepta (65535
// bytes): se manda, se recibe y tiene que volver idéntica, byte por byte.
TEST(ProtocolMockTest, CadenaDeLongitudMaxima) {
    MockSocket socket;
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);
    const std::string text(65535, 'x');
    // {7, 0xFF, 0xFF, 'x' x65535}: tipo 7 (Obtained) + largo de la cadena en
    // 2 bytes orden de red (0xFFFF = 65535, el máximo) + el contenido.
    std::vector<uint8_t> expected{7, 255, 255};
    expected.insert(expected.end(), text.begin(), text.end());

    sender.send_server_message(ServerObtained{text});
    EXPECT_EQ(socket.sent_bytes(), expected);
    socket.set_input(expected);
    EXPECT_EQ(std::get<ServerObtained>(receiver.receive_server_message()).content_id, text);
}

// ServerCrop junta todos los tipos de dato del protocolo en un solo mensaje
// (enteros de 16 bits, cadena, entero corto y booleanos): si alguno se
// serializa o lee en el lugar equivocado, este test lo atrapa.
TEST(ProtocolMockTest, MensajeConDistintosTiposDeDatos) {
    MockSocket socket;
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);
    // {19, 0x1234, 0xABCD, 0x0006 "potato", 3, 1, 0, 2} leído campo por campo:
    //   19        = tipo de mensaje (Crop)
    //   0x1234    = x (2 bytes, orden de red)
    //   0xABCD    = y (2 bytes)
    //   0, 6      = largo de crop_id (2 bytes) seguido de "potato" (6 chars)
    //   3         = stage (crecimiento)
    //   1         = needs_water (true)
    //   0         = interactive (false)
    //   2         = quality
    const std::vector<uint8_t> expected{
            19, 0x12, 0x34, 0xab, 0xcd, 0, 6, 'p', 'o', 't', 'a', 't', 'o', 3, 1, 0, 2};

    sender.send_server_message(ServerCrop{0x1234, 0xabcd, "potato", 3, true, false, 2});
    EXPECT_EQ(socket.sent_bytes(), expected);
    socket.set_input(expected);
    const auto crop = std::get<ServerCrop>(receiver.receive_server_message());
    EXPECT_EQ(crop.x, 0x1234);
    EXPECT_EQ(crop.y, 0xabcd);
    EXPECT_EQ(crop.crop_id, "potato");
    EXPECT_EQ(crop.stage, 3);
    EXPECT_TRUE(crop.needs_water);
    EXPECT_FALSE(crop.interactive);
    EXPECT_EQ(crop.quality, 2);
}

// Caso intermedio a propósito: un mensaje que no está en ningún extremo de
// longitud (id numérico + cadena corta), para no testear solo los bordes.
TEST(ProtocolMockTest, MensajeDeLongitudIntermedia) {
    MockSocket socket;
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);
    // {32, 7, 0x000A, "strawberry"}:
    //   32     = tipo de mensaje (PlayerHeld)
    //   7      = player_id
    //   0, 10  = largo de content_id (2 bytes) seguido del texto (10 chars)
    const std::vector<uint8_t> expected{
            32, 7, 0, 10, 's', 't', 'r', 'a', 'w', 'b', 'e', 'r', 'r', 'y'};

    sender.send_server_message(ServerPlayerHeld{7, "strawberry"});
    EXPECT_EQ(socket.sent_bytes(), expected);
    socket.set_input(expected);
    const auto held = std::get<ServerPlayerHeld>(receiver.receive_server_message());
    EXPECT_EQ(held.player_id, 7);
    EXPECT_EQ(held.content_id, "strawberry");
}

// 65536 bytes es una de más que el máximo: el sender tiene que rechazarlo
// con length_error antes de escribir un solo byte en el socket.
TEST(ProtocolMockTest, RechazaCadenaQueSuperaElLimite) {
    MockSocket socket;
    ProtocolSender sender(socket);
    EXPECT_THROW(sender.send_server_message(ServerObtained{std::string(65536, 'x')}),
                 std::length_error);
}

// El byte 255 no corresponde a ningún tipo de mensaje de servidor: el
// receiver tiene que tirar en vez de intentar leer un payload que no existe.
TEST(ProtocolMockTest, RechazaTipoDeMensajeDesconocido) {
    MockSocket socket;
    ProtocolReceiver receiver(socket);
    socket.set_input({255});
    EXPECT_THROW(receiver.receive_server_message(), std::invalid_argument);
}

// Dos situaciones: 
// no llegar a mandar nada significa "se cortó la conexión" (devuelve vacío), 
// y recibir un mensaje a medias es un error de verdad (tira excepción).
TEST(ProtocolMockTest, DistingueFinDeConexionDeMensajeIncompleto) {
    MockSocket socket;
    ProtocolReceiver receiver(socket);
    EXPECT_FALSE(receiver.receive_command().has_value());

    // {6, 0, 4, 'h', 'o'}: Chat (tipo 6) que dice tener 4 bytes de texto pero
    // sólo llegaron 2 ("ho"). Faltan dos: es un error, no un fin de conexión.
    socket.set_input({6, 0, 4, 'h', 'o'});
    EXPECT_THROW(receiver.receive_command(), std::exception);
}

// Recorre los 9 comandos que un cliente puede mandar. Para cada uno: se
// serializa y tiene que dar los bytes esperados, se lee de vuelta, y lo que
// salga del receiver se vuelve a serializar para confirmar que el
// ida-y-vuelta es exacto (si el orden de los campos cambiara, se nota acá).
TEST(ProtocolMockTest, LosNueveComandosDeClienteMantienenSuFormato) {
    struct CasoDeComando {
        const char* nombre;
        std::vector<uint8_t> bytes;
        protocol::ClientMessage mensaje;
    };
    // Cada caso: el comentario de arriba decodifica los bytes de la línea
    // siguiente, en el mismo orden en que los escribe el sender.
    // Direcciones: UP=0, DOWN=1, LEFT=2, RIGHT=3.
    const std::vector<CasoDeComando> casos{
            // Move: tipo 1 + dirección LEFT (2)
            {"Move", {1, 2}, protocol::MoveMessage{Direction::LEFT}},
            // Pick: tipo 2 + dirección DOWN (1) + slot 7
            {"Pick", {2, 1, 7}, protocol::PickMessage{Direction::DOWN, 7}},
            // Take: tipo 3 + dirección RIGHT (3) + slot 9
            {"Take", {3, 3, 9}, protocol::TakeMessage{Direction::RIGHT, 9}},
            // Sleep: tipo 4 + accept 1 (true = acepta dormir)
            {"Sleep", {4, 1}, protocol::SleepMessage{true}},
            // Wake: tipo 5, sin payload
            {"Wake", {5}, protocol::WakeMessage{}},
            // Chat: tipo 6 + largo 0x0002 (2 bytes) + el texto "hi"
            {"Chat", {6, 0, 2, 'h', 'i'}, protocol::ChatMessage{"hi"}},
            // GiftResponse: tipo 7 + sender_id 4 + accept 0 (false = rechaza)
            {"GiftResponse", {7, 4, 0}, protocol::GiftResponseMessage{4, false}},
            // ToggleWalking: tipo 8, sin payload
            {"ToggleWalking", {8}, protocol::ToggleWalkingMessage{}},
            // SelectSlot: tipo 9 + slot 6
            {"SelectSlot", {9, 6}, protocol::SelectSlotMessage{6}},
    };

    for (const CasoDeComando& caso: casos) {
        MockSocket socket;
        ProtocolSender sender(socket);
        ProtocolReceiver receiver(socket);

        sender.send_client_message(caso.mensaje);
        EXPECT_EQ(socket.sent_bytes(), caso.bytes) << caso.nombre;

        socket.set_input(caso.bytes);
        const auto command = receiver.receive_command();
        ASSERT_TRUE(command.has_value()) << caso.nombre;
        EXPECT_EQ(command->index(), caso.mensaje.index()) << caso.nombre;

        // Re-serializar lo recibido tiene que dar exactamente los mismos
        // bytes: si el receiver leyó los campos en otro orden, se nota acá.
        MockSocket reenvio;
        ProtocolSender reenviador(reenvio);
        reenviador.send_client_message(*command);
        EXPECT_EQ(reenvio.sent_bytes(), caso.bytes) << caso.nombre;
    }
}

// Ningún byte fuera del rango válido (0 o 255) debería pasar por el receiver
// como si fuera un comando de cliente: hay que rechazarlos.
TEST(ProtocolMockTest, RechazaComandoDeClienteDesconocido) {
    MockSocket socket;
    ProtocolReceiver receiver(socket);

    // {0}: el tipo 0 nunca existió.
    socket.set_input({0});
    EXPECT_THROW(receiver.receive_command(), std::invalid_argument);

    // {255}: no corresponde a ninguno de los comandos definidos.
    socket.set_input({255});
    EXPECT_THROW(receiver.receive_command(), std::invalid_argument);
}

// La validación de campos es de doble vía: nuestro sender no deja salir un
// mensaje inválido, y el receiver no acepta uno que venga así de un cliente
// que no pasa por nuestro código.
TEST(ProtocolMockTest, RechazaCamposInvalidosDelLadoDelCliente) {
    MockSocket socket;
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);

    // El sender tampoco deja salir una dirección que el receiver rechaza.
    EXPECT_THROW(sender.send_client_message(protocol::MoveMessage{static_cast<Direction>(4)}),
                 std::invalid_argument);
    EXPECT_TRUE(socket.sent_bytes().empty());

    // {1, 4}: Move (tipo 1) con dirección 4. Las válidas son 0..3, así que
    // esto es lo que mandaría un cliente que no usa nuestro sender.
    socket.set_input({1, 4});
    EXPECT_THROW(receiver.receive_command(), std::invalid_argument);

    // {4, 2}: Sleep (tipo 4) con accept = 2; sólo se aceptan 0 y 1.
    socket.set_input({4, 2});
    EXPECT_THROW(receiver.receive_command(), std::invalid_argument);

    // {7, 1, 2}: GiftResponse (tipo 7) con sender_id 1 y accept = 2.
    socket.set_input({7, 1, 2});
    EXPECT_THROW(receiver.receive_command(), std::invalid_argument);
}

TEST(ProtocolMockTest, OfertaDelMercadoConservaLaCalidadYRechazaValoresInvalidos) {
    MockSocket socket;
    ProtocolSender sender(socket);
    ProtocolReceiver receiver(socket);
    sender.send_server_message(ServerMarketOffer{"pierre", {
            ServerMarketEntry{"strawberry", "frutilla", ServerMarketSide::Buys, true, 50, 3},
            ServerMarketEntry{"egg", "huevo", ServerMarketSide::Buys, true, 3, 0}}});
    auto bytes = socket.sent_bytes();
    socket.set_input(bytes);
    const auto message = receiver.receive_server_message();
    const auto* offer = std::get_if<ServerMarketOffer>(&message);
    ASSERT_NE(offer, nullptr);
    ASSERT_EQ(offer->entries.size(), 2u);
    EXPECT_EQ(offer->entries[0].quality, 3);
    EXPECT_EQ(offer->entries[1].quality, 0);

    bytes.back() = 6;
    socket.set_input(bytes);
    EXPECT_THROW(receiver.receive_server_message(), std::invalid_argument);
}
