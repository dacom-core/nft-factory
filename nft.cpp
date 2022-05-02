
#include "nft.hpp"

/**
\defgroup public_consts CONSTS
\brief Константы контракта
*/

/**
\defgroup public_actions ACTIONS
\brief Методы действий контракта
*/

/**
\defgroup public_tables TABLES
\brief Структуры таблиц контракта
*/

  uint64_t nft::get_global_id(eosio::name key) {

    counts_index counts(_me, _me.value);
    auto count = counts.find(key.value);
    uint64_t id = 1;

    if (count == counts.end()) {
      counts.emplace(_me, [&](auto &c){
        c.key = "total"_n;
        c.value = id;
      });
    } else {
      id = count -> value + 1;
      counts.modify(count, _me, [&](auto &c){
        c.value = id;
      });
    }

    return id;

  }


void nft::add_balance(eosio::name payer, eosio::asset quantity, eosio::name contract){
    require_auth(payer);

    balances_index balances(_me, payer.value);
    
    auto balances_by_contract_and_symbol = balances.template get_index<"byconsym"_n>();
    auto contract_and_symbol_index = combine_ids(contract.value, quantity.symbol.code().raw());

    auto balance = balances_by_contract_and_symbol.find(contract_and_symbol_index);

    if (balance  == balances_by_contract_and_symbol.end()){
      balances.emplace(_me, [&](auto &b) {
        b.id = balances.available_primary_key();
        b.contract = contract;
        b.quantity = quantity;
      }); 
    } else {
      balances_by_contract_and_symbol.modify(balance, _me, [&](auto &b) {
        b.quantity += quantity;
      });
    };
  
}



void nft::sub_balance(eosio::name username, eosio::asset quantity, eosio::name contract){
    balances_index balances(_me, username.value);
    
    auto balances_by_contract_and_symbol = balances.template get_index<"byconsym"_n>();
    auto contract_and_symbol_index = combine_ids(contract.value, quantity.symbol.code().raw());

    auto balance = balances_by_contract_and_symbol.find(contract_and_symbol_index);
    
    eosio::check(balance != balances_by_contract_and_symbol.end(), "Balance is not found");
    
    eosio::check(balance -> quantity >= quantity, "Not enought user balance for create order");

    if (balance -> quantity == quantity) {

      balances_by_contract_and_symbol.erase(balance);

    } else {

      balances_by_contract_and_symbol.modify(balance, _me, [&](auto &b) {
        b.quantity -= quantity;
      });  

    }
    
}


  [[eosio::action]] void nft::create(eosio::name creator, eosio::name lang, std::string title, std::string description, eosio::name category, std::string images, std::string ipns, bool can_creator_split_to_pieces, bool can_creator_emit_pieces, std::string meta){
    
    require_auth(creator);
    
    objects_index objects(_me, _me.value);


    objects.emplace(_me, [&](auto &o) { 
      o.id = get_global_id("object"_n);
      o.creator = creator;
      o.owner = creator;
      o.lang = lang;
      o.title = title;
      o.description = description;
      o.category = category;
      o.images = images;
      o.ipns = ipns;
      o.total_pieces = 1;
      o.remain_pieces = 1;
      o.can_creator_split_to_pieces = can_creator_split_to_pieces;
      o.can_creator_emit_pieces = can_creator_emit_pieces;
      o.meta = meta;
    });


  }


  [[eosio::action]] void nft::remove(eosio::name owner, uint64_t id) {
    require_auth(owner);
    
    objects_index objects(_me, owner.value);

    auto object = objects.find(id);
    eosio::check(object != objects.end(), "NFT is not found");
    eosio::check(object -> owner == owner, "Only owner can remove this NFT");

    objects.erase(object);

  }


  [[eosio::action]] void nft::edit(eosio::name owner, uint64_t id, std::string title, std::string description, std::string images, std::string ipns, eosio::name category, bool can_creator_split_to_pieces, bool can_creator_emit_pieces, std::string meta) {
    require_auth(owner);
    
    objects_index objects(_me, _me.value);
    
    auto object = objects.find(id);  
    eosio::check(object != objects.end(), "NFT is not found");
    eosio::check(owner == object -> creator, "Only owner can edit NFT for now");
    
    //TODO 
    //Новые владельцы редактировать цену продажи
    //Если эмиссия частей разрешена создателем, то владелец может её запретить при новой передаче
    //Если разделение на части разрешено создателем, то владелец может его запретить при новой передаче
    
    //Кейсы: 
    // - спуск по цепочке оптовых дилеров, где на этапе розничной продажи запрещается дальнейшее разделение продукта
    // - сбор частей NFT от поставщиков с дальнейшей перепродажей оптовым дилерам без возможности эмиссии для дилеров


    //creator CAN only disable splitting
    
    if (can_creator_split_to_pieces == true ) {
      eosio::check(object -> can_creator_split_to_pieces == true, "This NFT already prohibit splitting to pieces for creators");  
    };
    
    //creator CAN only disable emitting
    if (can_creator_emit_pieces == true ) {
      eosio::check(object -> can_creator_emit_pieces == true, "This NFT already prohibit emitting pieces for creators");  
    };

    objects.modify(object, owner, [&](auto &o) {
      o.title = title;
      o.description = description;
      o.images = images;
      o.ipns = ipns;
      o.category = category;
      o.can_creator_split_to_pieces = can_creator_split_to_pieces;
      o.can_creator_emit_pieces = can_creator_emit_pieces;
    });

  }




  [[eosio::action]] void nft::sell(eosio::name owner, uint64_t object_id, uint64_t pieces_to_sell, eosio::asset one_piece_price, eosio::asset total_price, eosio::name pay_type, bool buyer_can_offer_price, bool with_delivery, eosio::name token_contract, eosio::name delivery_method, eosio::name delivery_operator, std::string delivery_details, eosio::asset delivery_fee, std::string meta) {
    require_auth(owner);

      

    objects_index objects(_me, _me.value);
    auto object = objects.find(object_id);  
    eosio::check(object != objects.end(), "NFT is not found");
    eosio::check(object -> owner == owner, "Only owner can sell NFT");
    eosio::check(pieces_to_sell <= object -> remain_pieces, "Not enough pieces for sell");
   
    market_index markets(_me, _me.value);

    markets.emplace(owner, [&](auto &o) {
      o.id = get_global_id("market"_n);
      o.object_id = object_id;
      o.seller = owner;
      o.lang = object -> lang;
      o.remain_pieces = pieces_to_sell;
      o.one_piece_price = one_piece_price;
      o.total_price = total_price;
      o.buyer_can_offer_price = buyer_can_offer_price;
      o.with_delivery = with_delivery;
      o.token_contract = token_contract;
      o.delivery_method = delivery_method;
      o.delivery_operator = delivery_operator;
      o.delivery_details = delivery_details;
      o.delivery_fee = delivery_fee;
      o.meta = meta;
    });

    uint64_t remain_pieces = object -> remain_pieces - pieces_to_sell;;

    objects.modify(object, owner, [&](auto &o) {
      o.remain_pieces = remain_pieces;
    });

  }


  // [[eosio::action]] void nft::delegate(eosio::name seller, uint64_t id){


  // }


  // [[eosio::action]] void nft::emit(eosio::name seller, uint64_t id){


  // }


  // [[eosio::action]] void nft::split(eosio::name seller, uint64_t id){


  // }


  [[eosio::action]] void nft::buy(uint64_t nft_id, eosio::name buyer, eosio::name lang, uint64_t requested_pieces, eosio::name token_contract, eosio::asset my_total_price, eosio::asset my_one_piece_price, std::string delivery_to, std::string meta){
    
    require_auth(buyer);

  }

  [[eosio::action]] void nft::sendmessage(uint64_t order_id, eosio::name lang, eosio::name username, eosio::name message){
    
    require_auth(username);
    //TODO check username buyer or seller

  }


extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == nft::_me.value) {
          
          if (action == "create"_n.value) {
            execute_action(name(receiver), name(code), &nft::create);
          } else if (action == "remove"_n.value) {
            execute_action(name(receiver), name(code), &nft::remove);
          } else if (action == "edit"_n.value) {
            execute_action(name(receiver), name(code), &nft::edit);
          } else if (action == "sell"_n.value) {
            execute_action(name(receiver), name(code), &nft::sell);
          } else if (action == "buy"_n.value) {
            execute_action(name(receiver), name(code), &nft::buy);
          } else if (action == "sendmessage"_n.value) {
            execute_action(name(receiver), name(code), &nft::sendmessage);
          }

        } else {

          if (action == "transfer"_n.value) {
            
            struct transfer{
                eosio::name from;
                eosio::name to;
                eosio::asset quantity;
                std::string memo;
            };

            auto op = eosio::unpack_action_data<transfer>();

            if (op.to == nft::_me) {
               nft::add_balance(op.from, op.quantity, eosio::name(code));  
            }
          }
        }
  };
};
