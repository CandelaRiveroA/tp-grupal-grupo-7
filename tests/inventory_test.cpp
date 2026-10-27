#include <gtest/gtest.h>
#include <memory>
// Ajusta estas rutas según dónde hayas puesto los archivos
#include "../server/domain/inventory/inventory.h"
#include "../server/domain/inventory/product.h"
#include "../server/domain/inventory/item_types.h"

// ---------------------------------------------------------
// TEST 1: Límite del inventario (Inventario lleno rechaza nuevos)
// ---------------------------------------------------------
TEST(InventoryTest, AgregarItemsDistintosRespetaElLimite) {
    Inventory inv(2); // Capacidad máxima de 2 slots
    
    auto item1 = std::make_unique<Product>(ItemID::STRAWBERRY, 50, Quality::BASE);
    auto item2 = std::make_unique<Product>(ItemID::LAPIS_ORB, 150, Quality::BASE);
    auto item3 = std::make_unique<Product>(ItemID::PUMPKIN, 200, Quality::BASE);

    // Los dos primeros entran en slots distintos
    EXPECT_TRUE(inv.add_item(std::move(item1)));
    EXPECT_TRUE(inv.add_item(std::move(item2)));
    
    // Verificamos que ocuparon los slots correctos
    EXPECT_FALSE(inv.is_slot_empty(0));
    EXPECT_FALSE(inv.is_slot_empty(1));
    
    // El tercero rebota
    EXPECT_FALSE(inv.add_item(std::move(item3)));
}
// ---------------------------------------------------------
// TEST 2: Productos iguales de misma calidad se apilan
// ---------------------------------------------------------
TEST(InventoryTest, ProductosIgualesMismaCalidadSeApilan) {
    Inventory inv(3); // Varios slots disponibles
    
    auto orb1 = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::BASE);
    auto orb2 = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::BASE);
    
    EXPECT_TRUE(inv.add_item(std::move(orb1)));
    EXPECT_TRUE(inv.add_item(std::move(orb2)));
    
    // Verificamos que se apilaron en el primer slot y los demás siguen vacíos
    EXPECT_FALSE(inv.is_slot_empty(0));
    EXPECT_TRUE(inv.is_slot_empty(1));
    EXPECT_TRUE(inv.is_slot_empty(2));
    
    // Verificamos la cantidad
    EXPECT_EQ(inv.get_amount(0), 2);
}
// ---------------------------------------------------------
// TEST 3: Productos iguales de DISTINTA calidad NO se apilan
// ---------------------------------------------------------
TEST(InventoryTest, ProductosIgualesDistintaCalidadNoSeApilan) {
    Inventory inv(3);
    
    auto orb_base = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::BASE);
    auto orb_perfecta = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::PERFECT);

    EXPECT_TRUE(inv.add_item(std::move(orb_base)));
    EXPECT_TRUE(inv.add_item(std::move(orb_perfecta)));
    
    // Verificamos que ocuparon slots separados (0 y 1)
    EXPECT_FALSE(inv.is_slot_empty(0));
    EXPECT_FALSE(inv.is_slot_empty(1));
    EXPECT_TRUE(inv.is_slot_empty(2));
    
    // Verificamos que las cantidades son 1 y 1
    EXPECT_EQ(inv.get_amount(0), 1);
    EXPECT_EQ(inv.get_amount(1), 1);
    
    // Obtenemos el ítem (sabemos que no es nulo)
    const Item* item0 = inv.peek_item(0);

    // Lo casteamos a Product de forma segura
    const Product* prod0 = dynamic_cast<const Product*>(item0);

    // Verificamos que el cast fue exitoso (es decir, que efectivamente era un Product)
    ASSERT_NE(prod0, nullptr); 

    // Ahora sí, llamamos a get_quality() sobre el Product
    EXPECT_EQ(prod0->get_quality(), Quality::BASE);
}

// ---------------------------------------------------------
// TEST 4: Límite de Stack (No pasa de 50 si no hay más slots)
// ---------------------------------------------------------
TEST(InventoryTest, ApilarMismoItemRespetaTopeYFallaSiNoHayMasSlots) {
    Inventory inv(1); // SOLO 1 SLOT
    
    // Metemos 50 orbes
    for (int i = 0; i < 50; ++i) {
        auto orb = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::BASE);
        EXPECT_TRUE(inv.add_item(std::move(orb)));
    }
    
    // El slot 0 debe tener exactamente 50
    EXPECT_EQ(inv.get_amount(0), 50);

    // Intentamos meter el 51
    auto orb51 = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::BASE);
    
    // Falla porque el slot 0 está lleno y no hay slot 1
    EXPECT_FALSE(inv.add_item(std::move(orb51)));
    
    // Verificamos que la cantidad no pasó de 50
    EXPECT_EQ(inv.get_amount(0), 50);
}
// ---------------------------------------------------------
// TEST 5: Límite de Stack con slot libre (Pasa de 50 creando un nuevo stack)
// ---------------------------------------------------------
TEST(InventoryTest, ApilarMismoItemPasaAlSiguienteSlotSiElPrimeroEstaLleno) {
    Inventory inv(2); // AHORA HAY 2 SLOTS
    
    // Metemos 50 orbes
    for (int i = 0; i < 50; ++i) {
        auto orb = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::BASE);
        EXPECT_TRUE(inv.add_item(std::move(orb)));
    }
    
    // El slot 0 debe tener 50
    EXPECT_EQ(inv.get_amount(0), 50);
    EXPECT_TRUE(inv.is_slot_empty(1));

    // Metemos la orbe 51
    auto orb51 = std::make_unique<Product>(ItemID::SILVER_ORB, 50, Quality::BASE);
    
    // ¡Ahora SÍ debe tener éxito!
    EXPECT_TRUE(inv.add_item(std::move(orb51)));
    
    // Verificamos que el slot 0 sigue en 50, y el slot 1 tiene 1
    EXPECT_EQ(inv.get_amount(0), 50);
    EXPECT_FALSE(inv.is_slot_empty(1));
    EXPECT_EQ(inv.get_amount(1), 1);
    
    // Verificamos que ambos slots tienen el mismo tipo de item
    EXPECT_EQ(inv.peek_item(0)->get_id(), ItemID::SILVER_ORB);
    EXPECT_EQ(inv.peek_item(1)->get_id(), ItemID::SILVER_ORB);
}