#include <algorithm>
#include <cmath>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/contract.hpp>
#include <eosio/action.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <eosio/datastream.hpp>

using namespace eosio;



class [[eosio::contract]] nft : public eosio::contract {

public:
    nft( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds)
    {}
    void apply(uint64_t receiver, uint64_t code, uint64_t action);   
    [[eosio::action]] void createnft(eosio::name creator, eosio::name lang, std::string title, std::string description, eosio::name category, std::string images, std::string ipns, eosio::name token_contract, bool can_creator_split_to_pieces, bool can_creator_emit_pieces, bool can_owner_split_to_pieces, bool can_owner_emit_pieces, eosio::asset one_piece_price, std::string meta);
    [[eosio::action]] void removenft(eosio::name owner, uint64_t id);
    [[eosio::action]] void editnft(eosio::name creator, uint64_t id, std::string title, std::string description, std::string images, std::string ipns, eosio::name category, eosio::asset one_piece_price, bool can_creator_split_to_pieces, bool can_creator_emit_pieces, bool can_owner_split_to_pieces, bool can_owner_emit_pieces);
   
    static void add_balance(eosio::name username, eosio::asset quantity, uint64_t code);
    static void sub_balance(eosio::name username, eosio::asset quantity, uint64_t code);


    [[eosio::action]] void createorder(uint64_t nft_id, eosio::name buyer, eosio::name lang, uint64_t requested_pieces, eosio::name token_contract, eosio::asset my_total_price, eosio::asset my_one_piece_price, std::string delivery_to, std::string meta);
    
    static uint64_t get_global_nft_id();
    static void payorder(eosio::name buyer, uint64_t order_id, eosio::name code, eosio::asset amount);
    [[eosio::action]] void sendmessage(uint64_t order_id, eosio::name lang, eosio::name username, eosio::name message);

    // [[eosio::action]] void acceptorder();
    // [[eosio::action]] void confirmsend();


    static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    };

      // std::string meta;     /*!< мета-данные объекта */

    

    /**
     * Статусы NFT на продаже:
     * waiting  - ожидание ордера
     * process - ожидание оплаты
     * payed - ожидание подтверждения отправки
     */

    /**
    * @ingroup public_consts 
    * @{ 
    */
    static constexpr eosio::name _me = "nft"_n;   
    static constexpr bool _anyone_can_sell = false;

    static constexpr eosio::symbol _SYM     = eosio::symbol(eosio::symbol_code("FLOWER"), 4);
    static constexpr eosio::name _core_token_contract = "eosio.token"_n;

    /**
    * @}
    */
    
};


    /**
     * @brief      Таблица хранения объектов
     * @contract _me
     * @scope nft | buyer
     * @table objects
     * @ingroup public_tables
     * @details Таблица хранит объекты 
     */
    struct [[eosio::table, eosio::contract("nft")]] objects {
      uint64_t id;          /*!< идентификатор объекта */
      eosio::name creator;   /*!< создатель */
      eosio::name owner;   /*!< продавец */
      eosio::name lang;     /*!< языковой код объекта */
      std::string title;    /*!< название объекта */
      std::string description; /*!< краткое описание объекта */
      eosio::name category; /*!< категория объекта */
      std::string images;   /*!< изображения объекта */
      std::string ipns; /*!< контейнер объекта */
      bool can_creator_split_to_pieces = true;
      bool can_creator_emit_pieces = true;

      bool can_owner_split_to_pieces = true;
      bool can_owner_emit_pieces = true;
      
      
      uint64_t total_pieces; /*!< количество частей */
      uint64_t remain_pieces; /*!< сколько частей осталось */
      
      bool buyer_can_offer_price = false; /*!< может ли покупатель предлагать свою цену */
      bool with_delivery = false;         /*!< флаг физической поставки */

      eosio::name delivery_method;        /*!< метод физической поставки */
      eosio::name delivery_operator;        /*!< оператор физической поставки */
      std::string delivery_details;       /*!< детали физической поставки */
      eosio::asset delivery_fee;          /*!< цена физической поставки от оператора */
      
      eosio::name token_contract;         /*!< контракт токена обмена */
      eosio::asset creator_base_price;    /*!< цена создателя за одну часть */
      eosio::asset new_owner_price;       /*!< цена нового владельца за одну часть */
      eosio::asset one_piece_price;       /*!< цена за одну часть */

      std::string meta;     /*!< мета-данные объекта */

      uint64_t primary_key() const {return id;}       /*!< return id - primary_key */
      uint64_t byowner() const {return owner.value; }   /*!< return lang - secondary_key 2 */
      uint64_t bycreator() const {return creator.value; }   /*!< return lang - secondary_key 2 */
      uint64_t bycategory() const {return category.value; }   /*!< return lang - secondary_key 2 */
      uint64_t bydop() const {return delivery_operator.value; }   /*!< return lang - secondary_key 2 */
      
      uint64_t bylang() const {return lang.value; }   /*!< return lang - secondary_key 3 */
      uint64_t bydmethod() const {return delivery_method.value; }   /*!< return lang - secondary_key 3 */

      EOSLIB_SERIALIZE(objects, (id)(creator)(owner)(lang)(description)(category)(images)(ipns)(can_creator_split_to_pieces)(can_creator_emit_pieces)(can_owner_split_to_pieces)(can_owner_emit_pieces)(total_pieces)(remain_pieces)(buyer_can_offer_price)(with_delivery)(delivery_method)(delivery_operator)(delivery_details)(delivery_fee)(token_contract)(creator_base_price)(new_owner_price)(one_piece_price)(meta))
    };

    typedef eosio::multi_index< "objects"_n, objects,
      eosio::indexed_by<"byowner"_n, eosio::const_mem_fun<objects, uint64_t, &objects::byowner>>,
      eosio::indexed_by<"bycategory"_n, eosio::const_mem_fun<objects, uint64_t, &objects::bycategory>>,
      eosio::indexed_by<"bylang"_n, eosio::const_mem_fun<objects, uint64_t, &objects::bylang>>,
      eosio::indexed_by<"bydmethod"_n, eosio::const_mem_fun<objects, uint64_t, &objects::bydmethod>>,
      eosio::indexed_by<"bydop"_n, eosio::const_mem_fun<objects, uint64_t, &objects::bydop>>
      
    > objects_index;



/**
     * @brief      Таблица хранения предложений
     * @contract _me
     * @scope nft | buyer
     * @table objects
     * @ingroup public_tables
     * @details Таблица хранит категории событий хоста. 
     */
    struct [[eosio::table, eosio::contract("nft")]] orders {
      uint64_t id;          /*!< идентификатор предложения */
      uint64_t nft_id;          /*!< идентификатор объекта */
      eosio::name seller;   /*!< продавец */
      eosio::name buyer;   /*!< покупатель */
      eosio::name lang;     /*!< языковой код покупателя */
      
      eosio::name status;   /*!< статус предложения */
      
      uint64_t requested_pieces; /*!< количество частей */
      eosio::name token_contract; /*!< контракт токена обмена */
      
      eosio::asset my_total_price; /*!< общее предложение цены покупателя */
      eosio::asset my_one_piece_price;  /*!< предложение покупателя за одну часть*/
      eosio::asset total_payed;  /*!< предложение оплачено на сумму */
      std::string delivery_to;
      std::string meta;     /*!< мета-данные объекта */

      uint64_t primary_key() const {return id;}       /*!< return id - primary_key */
      uint64_t bynft() const {return nft_id; }   /*!< return nft_id - secondary_key 2 */
      uint64_t bylang() const {return lang.value; }   /*!< return lang - secondary_key 3 */

      uint64_t byseller() const {return seller.value; }   /*!< return lang - secondary_key 4 */
      uint64_t bybuyer() const {return buyer.value; }   /*!< return lang - secondary_key 5 */
      uint64_t bycon() const {return token_contract.value; }   /*!< return contract - secondary_key 6 */


      EOSLIB_SERIALIZE(orders, (id)(nft_id)(seller)(buyer)(lang)(status)(requested_pieces)(token_contract)(my_total_price)(my_one_piece_price)(total_payed)(delivery_to)(meta))
    };

    typedef eosio::multi_index< "orders"_n, orders,
      eosio::indexed_by<"bynft"_n, eosio::const_mem_fun<orders, uint64_t, &orders::bynft>>,
      eosio::indexed_by<"bylang"_n, eosio::const_mem_fun<orders, uint64_t, &orders::bylang>>,
      eosio::indexed_by<"byseller"_n, eosio::const_mem_fun<orders, uint64_t, &orders::byseller>>,
      eosio::indexed_by<"bybuyer"_n, eosio::const_mem_fun<orders, uint64_t, &orders::bybuyer>>,
      eosio::indexed_by<"bycon"_n, eosio::const_mem_fun<orders, uint64_t, &orders::bycon>>
      
    > orders_index;




/**
     * @brief      Таблица хранения зашифрованного общения между покупателем и продавцом
     * @contract _me
     * @scope nft | buyer
     * @table messages
     * @ingroup public_tables
     * @details Таблица хранит категории событий хоста. 
     */
    struct [[eosio::table, eosio::contract("nft")]] messages {
      uint64_t id;
      uint64_t order_id;          /*!< идентификатор предложения */
      eosio::name lang;     /*!< языковой код ветки */
      eosio::name username;
      eosio::name message;

      std::string meta;     /*!< мета-данные объекта */

      uint64_t primary_key() const {return id;}       /*!< return id - primary_key */
      uint64_t byorder() const {return order_id; }   /*!< return order_id - secondary_key 2 */
      uint64_t byusername() const {return username.value; }   /*!< return username - secondary_key 3 */

      uint64_t bylang() const {return lang.value; }   /*!< return lang - secondary_key 4 */

      EOSLIB_SERIALIZE(messages, (id)(order_id)(lang)(username)(message)(meta))
    };

    typedef eosio::multi_index< "messages"_n, messages,
      eosio::indexed_by<"byorder"_n, eosio::const_mem_fun<messages, uint64_t, &messages::byorder>>,
      eosio::indexed_by<"byusername"_n, eosio::const_mem_fun<messages, uint64_t, &messages::byusername>>,
      eosio::indexed_by<"bylang"_n, eosio::const_mem_fun<messages, uint64_t, &messages::bylang>>
      
    > messages_index;



/**
     * @brief      Таблица хранения счётчиков NFT
     * @contract _me
     * @scope nft | nft
     * @table counts
     * @ingroup public_tables
     * @details Таблица хранит категории событий хоста. 
     */
    struct [[eosio::table, eosio::contract("nft")]] counts {
      eosio::name key;
      eosio::name secondary_key;
      uint64_t value;
      double   double_value;

      uint64_t primary_key() const {return key.value;}       /*!< return id - primary_key */
      uint128_t keyskey() const {return nft::combine_ids(key.value, secondary_key.value);} /*!< (contract, blocked_now.symbol) - комбинированный secondary_key для получения курса по имени выходного контракта и символу */
      uint128_t keyvalue() const {return nft::combine_ids(key.value, value);} /*!< (contract, blocked_now.symbol) - комбинированный secondary_key для получения курса по имени выходного контракта и символу */
      

      EOSLIB_SERIALIZE(counts, (key)(secondary_key)(value)(double_value))
    };

    typedef eosio::multi_index< "counts"_n, counts,
        eosio::indexed_by<"keyskey"_n, eosio::const_mem_fun<counts, uint128_t, &counts::keyskey>>,
        eosio::indexed_by<"keyvalue"_n, eosio::const_mem_fun<counts, uint128_t, &counts::keyvalue>>
    > counts_index;

    