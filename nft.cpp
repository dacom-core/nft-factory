
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



  [[eosio::action]] void nft::createnft(eosio::name seller, eosio::name lang, std::string title, std::string description, std::string category, std::string images, std::string ipns, uint64_t total_pieces, uint64_t remain_pieces, bool buyer_can_order_price, bool with_delivery, std::string delivery_method, eosio::name token_contract, eosio::asset total_price, eosio::asset one_piece_price, std::string meta){
    
    require_auth(seller);
    
    objects_index objects(_me, _me.value);

    //TODO check for NFT already exist in user memory or seller is _me
    if (_anyone_can_sell == false) {

      eosio::check(seller == _me, "Only owner of this contract can create the new NFT");
      
    }



  }



  [[eosio::action]] void nft::removenft() {
    //TODO delete if not have any solded pieces

    require_auth(seller);
    
    objects_index objects(_me, _me.value);
  

  }

  [[eosio::action]] void nft::editnft() {
    //TODO edit if not have any solded pieces

    require_auth(seller);
    
    objects_index objects(_me, _me.value);
  

  }


  [[eosio::action]] void nft::sellmynft(eosio::name seller, uint64_t id){


  }

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
          } else if (action == "creatorder"_n.value) {
            execute_action(name(receiver), name(code), &nft::createnft);
          } else if (action == "sendmessage"_n.value) {
            execute_action(name(receiver), name(code), &nft::sendmessage);
          }

        } else {

          if (action == "transfer"_n.value){
            
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
