
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

  uint64_t nft::get_global_nft_id() {

    counts_index counts(_me, _me.value);
    auto count = counts.find("total"_n.value);
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


  [[eosio::action]] void nft::createnft(eosio::name creator, eosio::name lang, std::string title, std::string description, eosio::name category, std::string images, std::string ipns, eosio::name token_contract, bool can_creator_split_to_pieces, bool can_creator_emit_pieces, bool can_owner_split_to_pieces, bool can_owner_emit_pieces, eosio::asset one_piece_price, std::string meta){
    
    require_auth(creator);
    
    objects_index objects(_me, creator.value);


    objects.emplace(_me, [&](auto &o) { 
      o.id = get_global_nft_id();
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
      o.can_owner_split_to_pieces = can_owner_split_to_pieces;
      o.can_owner_emit_pieces = can_owner_emit_pieces;
      o.token_contract = token_contract;
      o.creator_base_price = one_piece_price;
      o.new_owner_price = one_piece_price;
      o.one_piece_price = one_piece_price;
      o.meta = meta;
    });


  }


  [[eosio::action]] void nft::removenft(eosio::name owner, uint64_t id) {
    require_auth(owner);
    
    objects_index objects(_me, owner.value);

    auto object = objects.find(id);
    eosio::check(object != objects.end(), "NFT is not found");
    eosio::check(object -> owner == owner, "Only owner can remove this NFT");

    objects.erase(object);

  }


  [[eosio::action]] void nft::editnft(eosio::name owner, uint64_t id, std::string title, std::string description, std::string images, std::string ipns, eosio::name category, eosio::asset one_piece_price, bool can_creator_split_to_pieces, bool can_creator_emit_pieces, bool can_owner_split_to_pieces, bool can_owner_emit_pieces) {
    require_auth(owner);
    
    objects_index objects(_me, owner.value);
    
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

    //creator CAN only disable splitting for owners
    if (can_owner_split_to_pieces == true ) {
      eosio::check(object -> can_owner_split_to_pieces == true, "This NFT already prohibit splitting to pieces for owners");  
    };
    
    //creator CAN only disable emitting for owners
    if (can_owner_emit_pieces == true ) {
      eosio::check(object -> can_owner_emit_pieces == true, "This NFT already prohibit emitting pieces for owners");  
    };

    objects.modify(object, owner, [&](auto &o) {
      o.title = title;
      o.description = description;
      o.images = images;
      o.ipns = ipns;
      o.category = category;
      o.one_piece_price = one_piece_price;
      o.can_creator_split_to_pieces = can_creator_split_to_pieces;
      o.can_creator_emit_pieces = can_creator_emit_pieces;
      o.can_owner_split_to_pieces = can_owner_split_to_pieces;
      o.can_owner_emit_pieces = can_owner_emit_pieces;
    });

  }


  // [[eosio::action]] void nft::sellmynft(eosio::name seller, uint64_t id){


  // }

  // [[eosio::action]] void nft::emittomynft(eosio::name seller, uint64_t id){


  // }


  // [[eosio::action]] void nft::splitmynft(eosio::name seller, uint64_t id){


  // }


  [[eosio::action]] void nft::createorder(uint64_t nft_id, eosio::name buyer, eosio::name lang, uint64_t requested_pieces, eosio::name token_contract, eosio::asset my_total_price, eosio::asset my_one_piece_price, std::string delivery_to, std::string meta){
    
    require_auth(buyer);

  }

  [[eosio::action]] void nft::sendmessage(uint64_t order_id, eosio::name lang, eosio::name username, eosio::name message){
    
    require_auth(username);
    //TODO check username buyer or seller

  }


extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == nft::_me.value) {
          
          if (action == "createnft"_n.value) {
            execute_action(name(receiver), name(code), &nft::createnft);
          } else if (action == "removenft"_n.value) {
            execute_action(name(receiver), name(code), &nft::removenft);
          } else if (action == "editnft"_n.value) {
            execute_action(name(receiver), name(code), &nft::editnft);
          } else if (action == "creatorder"_n.value) {
            execute_action(name(receiver), name(code), &nft::createnft);
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
              //DISPATCHER FOR INCOME TRANSFERS 
            }
          }
        }
  };
};
