#include <gtest/gtest.h>
#include <memory>

#include "../server/domain/entities/player.h"
#include "../server/domain/inventory/product.h"
#include "../common/common_constants.h"
#include "../server/domain/inventory/item_types.h"

// ---------------------------------------------------------
// TEST 1: Transacciones de Oro
// ---------------------------------------------------------
TEST(PlayerEconomyTest, GoldTransactionsWorkCorrectly) {
    Player p(1, 0, 0);
    // El oro inicial es aleatorio entre 0 y 100[cite: 1, 16]
    uint16_t initial_gold = p.get_gold();

    // Sumar oro
    p.add_gold(50);
    EXPECT_EQ(p.get_gold(), initial_gold + 50);

    // Restar oro con saldo suficiente
    EXPECT_TRUE(p.try_remove_gold(20));
    EXPECT_EQ(p.get_gold(), initial_gold + 30);

    // Restar oro con saldo insuficiente (debe fallar y no modificar el saldo)
    EXPECT_FALSE(p.try_remove_gold(initial_gold + 100));
    EXPECT_EQ(p.get_gold(), initial_gold + 30);
}

// ---------------------------------------------------------
// TEST 2: Regalo Rechazado (Objeto Intransferible)
// ---------------------------------------------------------
TEST(PlayerGiftTest, CannotGiftNonTradableItem) {
    Player p1(1, 0, 0);
    Player p2(2, 0, 0);

    // El slot 0 contiene el pico (inicializado con tradable = false)[cite: 1]
    p1.select_slot(0);

    // La transferencia debe ser rechazada de inmediato
    EXPECT_FALSE(p1.finalize_gift_reception(p2));
    
    // Verificamos que el pico sigue en el inventario del Jugador 1
    EXPECT_FALSE(p1.get_inventory().is_slot_empty(0));
}

// ---------------------------------------------------------
// TEST 3: Regalo Exitoso
// ---------------------------------------------------------
TEST(PlayerGiftTest, SuccessfulGiftTransfersItem) {
    Player p1(1, 0, 0);
    Player p2(2, 0, 0);

    // Le damos al Jugador 1 un ítem transferible.
    // Caerá en el slot 8 porque los slots 0 al 7 están ocupados por el constructor[cite: 1]
    p1.pick_up_item(std::make_unique<Product>(ItemID::STRAWBERRY, 80, Quality::BASE));
    p1.select_slot(8); 

    EXPECT_TRUE(p1.finalize_gift_reception(p2));

    // Verificamos que el Jugador 1 perdió el ítem
    EXPECT_TRUE(p1.get_inventory().is_slot_empty(8));

    // Verificamos que el Jugador 2 lo recibió en su slot 8
    EXPECT_FALSE(p2.get_inventory().is_slot_empty(8));
    EXPECT_EQ(p2.get_inventory().peek_item(8)->get_id(), ItemID::STRAWBERRY);
}

// ---------------------------------------------------------
// TEST 4: Regalo Rechazado (Inventario del receptor lleno)
// ---------------------------------------------------------
TEST(PlayerGiftTest, GiftFailsIfReceiverInventoryIsFull) {
    Player p1(1, 0, 0);
    Player p2(2, 0, 0);

    // Llenamos el único slot libre (slot 8) del Jugador 2 con madera
    p2.pick_up_item(std::make_unique<Product>(ItemID::EGG, 80, Quality::BASE));
    
    // El Jugador 1 intenta regalarle una frutilla
    p1.pick_up_item(std::make_unique<Product>(ItemID::STRAWBERRY, 80, Quality::BASE));
    p1.select_slot(8);

    // La transferencia debe fallar porque el inventario del receptor está lleno
    EXPECT_FALSE(p1.finalize_gift_reception(p2));

    // Verificamos que el Jugador 1 NO perdió su frutilla (protección de memoria)
    EXPECT_FALSE(p1.get_inventory().is_slot_empty(8));
    EXPECT_EQ(p1.get_inventory().peek_item(8)->get_id(), ItemID::STRAWBERRY);
}
