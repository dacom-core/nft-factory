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
    [[eosio::action]] void createnft(eosio::name seller, eosio::name lang, std::string title, std::string description, std::string category, std::string images, std::string ipns, uint64_t total_pieces, uint64_t remain_pieces, bool buyer_can_order_price, bool with_delivery, std::string delivery_method, eosio::name token_contract, eosio::asset total_price, eosio::asset one_piece_price, std::string meta);
    
    [[eosio::action]] void createorder(uint64_t nft_id, eosio::name buyer, eosio::name lang, uint64_t requested_pieces, eosio::name token_contract, eosio::asset my_total_price, eosio::asset my_one_piece_price, std::string delivery_to, std::string meta);
    

    static void payorder(eosio::name buyer, uint64_t order_id, eosio::name code, eosio::asset amount);
    [[eosio::action]] void sendmessage(uint64_t order_id, eosio::name lang, eosio::name username, eosio::name message);

    // [[eosio::action]] void acceptorder();
    // [[eosio::action]] void confirmsend();



      std::string meta;     /*!< мета-данные объекта */

    

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
      eosio::name seller;   /*!< продавец */
      eosio::name lang;     /*!< языковой код объекта */
      std::string title;    /*!< название объекта */
      std::string description; /*!< краткое описание объекта */
      std::string category; /*!< категория объекта */
      std::string images;   /*!< изображения объекта */
      std::string ipns; /*!< контейнер объекта */
      
      uint64_t total_pieces; /*!< количество частей */
      uint64_t remain_pieces; /*!< сколько частей осталось */
      
      bool buyer_can_order_price = false;
      bool with_delivery = true;
      std::string delivery_method;

      eosio::name token_contract; /*!< контракт токена обмена */
      eosio::asset total_price; /*!< цена за все части */
      eosio::asset one_piece_price; /*!< цена за одну часть */

      std::string meta;     /*!< мета-данные объекта */

      uint64_t primary_key() const {return id;}       /*!< return id - primary_key */
      uint64_t byseller() const {return seller.value; }   /*!< return lang - secondary_key 2 */
      uint64_t bylang() const {return lang.value; }   /*!< return lang - secondary_key 3 */

      EOSLIB_SERIALIZE(objects, (id)(seller)(lang)(description)(category)(images)(ipns)(total_pieces)(remain_pieces)(buyer_can_order_price)(with_delivery)(delivery_method)(token_contract)(total_price)(one_piece_price)(meta))
    };

    typedef eosio::multi_index< "objects"_n, objects,
      eosio::indexed_by<"byseller"_n, eosio::const_mem_fun<objects, uint64_t, &objects::byseller>>,
      eosio::indexed_by<"bylang"_n, eosio::const_mem_fun<objects, uint64_t, &objects::bylang>>
      
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

    