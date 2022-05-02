
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
        c.key = key;
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


  [[eosio::action]] void nft::create(eosio::name creator, eosio::name lang, std::string title, std::string description, eosio::name category, std::string images, std::string ipns, bool creator_can_emit_new_pieces, std::string meta){
    
    require_auth(creator);
    
    objects_index objects(_me, _me.value);


    objects.emplace(_me, [&](auto &o) { 
      o.id = get_global_id("objects"_n);
      o.creator = creator;
      o.lang = lang;
      o.title = title;
      o.description = description;
      o.category = category;
      o.images = images;
      o.ipns = ipns;
      o.total_pieces = 1;
      o.remain_pieces = 1;
      o.creator_can_emit_new_pieces = creator_can_emit_new_pieces;
      o.meta = meta;
    });


  }


  [[eosio::action]] void nft::remove(eosio::name creator, uint64_t object_id) {
    require_auth(creator);
    
    objects_index objects(_me, creator.value);

    auto object = objects.find(object_id);
    eosio::check(object != objects.end(), "NFT is not found");
    eosio::check(object -> creator == creator, "Only creator can remove this NFT");
    eosio::check(object -> total_pieces == object -> remain_pieces, "Cannot delete NFT after sell pieces");

    objects.erase(object);

  }


  [[eosio::action]] void nft::edit(eosio::name owner, uint64_t object_id, std::string title, std::string description, std::string images, std::string ipns, eosio::name category, bool creator_can_emit_new_pieces, std::string meta) {
    require_auth(owner);
    
    objects_index objects(_me, _me.value);
    
    auto object = objects.find(object_id);  
    eosio::check(object != objects.end(), "NFT is not found");
    eosio::check(owner == object -> creator, "Only owner can edit NFT for now");
    
    
    //creator CAN only disable emitting | after disable cant enable
    if (creator_can_emit_new_pieces == true ) {
      eosio::check(object -> creator_can_emit_new_pieces == true, "This NFT already prohibit emitting pieces for creators");  
    };

    objects.modify(object, owner, [&](auto &o) {
      o.title = title;
      o.description = description;
      o.images = images;
      o.ipns = ipns;
      o.category = category;
      o.creator_can_emit_new_pieces = creator_can_emit_new_pieces;
    });

  }




  [[eosio::action]] void nft::sell(eosio::name creator, uint64_t object_id, uint64_t pieces_to_sell, eosio::asset one_piece_price, eosio::asset total_price, bool buyer_can_offer_price, bool with_delivery, eosio::name token_contract, std::string delivery_from, std::vector<eosio::name> delivery_methods, std::vector<eosio::name> delivery_operators, std::string meta) {
    require_auth(creator);      

    objects_index objects(_me, _me.value);
    auto object = objects.find(object_id);  
    eosio::check(object != objects.end(), "NFT is not found");
    eosio::check(object -> creator == creator, "Only creator can sell NFT");
    eosio::check(pieces_to_sell <= object -> remain_pieces, "Not enough pieces for sell");
   
    market_index markets(_me, _me.value);

    auto itr = std::find(delivery_methods.begin(), delivery_methods.end(), "selfdelivery"_n);

    eosio::check(itr != delivery_methods.end(), "Now only self-delivery method is possible");
    eosio::check(delivery_operators.size() == 0, "Now delivery operators is not accepted");

    markets.emplace(creator, [&](auto &o) {
      o.id = get_global_id("markets"_n);
      o.object_id = object_id;
      o.seller = creator;
      o.lang = object -> lang;
      o.status = "waiting"_n;
      o.remain_pieces = pieces_to_sell;
      o.one_piece_price = one_piece_price;
      o.total_price = total_price;
      o.buyer_can_offer_price = buyer_can_offer_price;
      o.with_delivery = with_delivery;
      o.token_contract = token_contract;
      o.delivery_from = delivery_from;
      o.delivery_methods = delivery_methods;
      o.delivery_operators = delivery_operators;      
      o.meta = meta;
    });

    uint64_t remain_pieces = object -> remain_pieces - pieces_to_sell;;

    objects.modify(object, owner, [&](auto &o) {
      o.remain_pieces = remain_pieces;
    });

  }



  [[eosio::action]] void nft::buy(eosio::name buyer, uint64_t market_id, eosio::name lang, uint64_t requested_pieces, eosio::asset total_price, eosio::asset one_piece_price, std::string delivery_to, eosio::name delivery_method, eosio::name delivery_operator, std::string meta){
    
    require_auth(buyer);

    market_index markets(_me, _me.value);
    auto market = markets.find(market_id);
    
    eosio::check(market != markets.end(), "Object is not found on the market");
    eosio::check(market -> remain_pieces >= requested_pieces, "Not enought pieces on the market");
    eosio::check(market -> total_price.symbol == total_price.symbol, "Symbols is not equal");
    eosio::check(market -> one_piece_price.symbol == one_piece_price.symbol, "Symbols is not equal");
    
    eosio::check(market -> status != "pause"_n, "Market on pause");

    if (market -> buyer_can_offer_price == false) {
      eosio::check(market -> total_price.amount == total_price.amount, "Buyer cant offer the price.");
      eosio::check(market -> one_piece_price.amount == one_piece_price.amount, "Buyer cant offer the price.");
    
      eosio::check(one_piece_price.amount * requested_pieces == total_price.amount, "Price for pieces and total price is not equal");

    
    } else {
      //TODO check offered price
    };

    sub_balance(buyer, total_price, market -> token_contract);


    objects_index objects(_me, _me.value);
    auto object = objects.find(market -> object_id);

    if (market -> with_delivery == true) {
      requests_index requests(_me, _me.value);
      
      requests.emplace(buyer, [&](auto &r) {
        r.id = get_global_id("requests"_n);
        r.market_id = market -> id;
        r.seller = market -> seller;
        r.buyer = buyer;
        r.lang = lang;
        r.requested_pieces = requested_pieces;
        r.total_price = total_price;
        r.one_piece_price = one_piece_price;
        r.total_payed = total_price;
        r.status = market -> buyer_can_offer_price == false ? "payed"_n : "waiting"_n;
        r.delivery_to = delivery_to;
        r.delivery_method = delivery_method;
        r.delivery_operator = delivery_operator;
        r.meta = meta;
      });

    } else {

      pieces_index pieces(_me, _me.value);
      auto piece = pieces.find(object -> id);

      if (piece == pieces.end()) {
        
        pieces.emplace(buyer, [&](auto &p) {
          p.object_id = object -> id;
          p.pieces = requested_pieces;
        });

      } else {

        pieces.modify(piece, buyer, [&](auto &p){
          p.pieces += requested_pieces;
        });

      }
    }


    if (market -> requested_pieces + requested_pieces == market->remain_pieces) {
      
      markets.erase(market);

    } else {

      markets.modify(market, buyer, [&](auto &o) {
        o.requested_pieces += requested_pieces;
      });

    }
  }



  [[eosio::action]] void nft::cancelsell(eosio::name creator, uint64_t market_id){
    
    require_auth(creator);

    market_index markets(_me, _me.value);
    auto market = markets.find(market_id);

    eosio::check(market -> seller == seller, "Only owner of market can cancel this sell");
    eosio::check(market -> blocked_pieces == 0, "Only markets with none blocked pieces can be canceled");

    
    objects_index objects(_me, _me.value);
    auto object = objects.find(market -> object_id);

    objects.modify(object, seller, [&](auto &o) {
      o.remain_pieces += object -> remain_pieces + market -> remain_pieces;
    });

    markets.erase(market);

  }

  [[eosio::action]] void nft::acceptreq(eosio::name seller, uint64_t request_id) {

    require_auth(seller);

    requests_index requests(_me, _me.value);
    auto request = requests.find(request_id);

    eosio::check(request != requests.end(), "Request is not found");
    eosio::check(request -> seller == seller, "Only seller can accept request");

    requests.modify(request, seller, [&](auto &r){
      r.status = "accepted"_n;
    });

  }
    


  [[eosio::action]] void nft::declinereq(eosio::name seller, uint64_t request_id) {

    require_auth(seller);

    requests_index requests(_me, _me.value);
    auto request = requests.find(request_id);

    eosio::check(request != requests.end(), "Request is not found");
    eosio::check(request -> seller == seller, "Only seller can decline request");
    requests.modify(request, seller, [&](auto &r){
      r.status = "declined"_n;
    });

  }

  [[eosio::action]] void nft::cancelreq(eosio::name buyer, uint64_t request_id){

    require_auth(buyer);

    requests_index requests(_me, _me.value);
    auto request = requests.find(request_id);

    eosio::check(request != requests.end(), "Request is not found");
    eosio::check(request -> buyer == buyer, "Only buyer can cancel request");
    eosio::check(request -> status == "waiting"_n, "Only requests on waiting status can be canceled");
    
    market_index markets(_me, _me.value);
    auto market = markets.find(request -> market_id);

    action(
        permission_level{ _me, "active"_n },
        market->token_contract, "transfer"_n,
        std::make_tuple( _me, buyer, request -> total_price, std::string("Cancel request")) 
    ).send();

    requests.erase(request);

  };


  [[eosio::action]] void nft::payrequest(eosio::name buyer, uint64_t request_id){

    require_auth(buyer);
    requests_index requests(_me, _me.value);
    auto request = requests.find(request_id);

    eosio::check(request != requests.end(), "Request is not found");
    eosio::check(request -> buyer == buyer, "Only buyer can pay for request");
    eosio::check(request -> status == "accepted"_n, "Only accepted requests can be payed");

    market_index markets(_me, _me.value);
    auto market = markets.find(request -> market_id);

    sub_balance(buyer, request -> total_price, market -> token_contract);

    requests.modify(request, buyer, [&](auto &r){
      r.status = "payed"_n;
    });

  }


  [[eosio::action]] void nft::setdelstatus(eosio::name delivery_operator, uint64_t request_id, eosio::name status){

    require_auth(delivery_operator);

    requests_index requests(_me, _me.value);
    auto request = requests.find(request_id);

    eosio::check(request != requests.end(), "Request is not found");
    eosio::check(request -> delivery_operator == request -> seller, "Only seller can confirm delivery for now");

    eosio::check(request -> status == "payed"_n, "Only payed requests can be delivered");
    eosio::check(status == "finish"_n, "Only finish status are accepted for now (cause self-delivery)");

    requests.modify(request, delivery_operator, [&](auto &r){
      r.status = "finish"_n;
    });

  }

  


  [[eosio::action]] void nft::emit(eosio::name username, uint64_t object_id, uint64_t pieces_for_emit){
  
    require_auth(username);
    
    objects_index objects(_me, _me.value);
    auto object = objects.find(object_id);

    eosio::check(object != objects.end(), "Cant find object");
    eosio::check(object -> creator_can_emit_new_pieces == true, "Prohibit emit new pieces");

    whitelist_index whitelist(_me, _me.value);
    auto whitelist_by_object_and_users = whitelist.template get_index<"byobjanduser"_n>();
    auto object_and_user = combine_ids(object_id, username.value);
    auto wl = whitelist_by_object_and_users.find(object_and_user);

    eosio::check(wl != whitelist_by_object_and_users.end() || object -> creator == username, "Only users from whitelist or creator can emit new pieces");

    objects.modify(object, username, [&](auto &o){
      o.total_pieces += pieces_for_emit;
      o.remain_pieces += pieces_for_emit;
    });
  
  }
  

  [[eosio::action]] void nft::addtowl(eosio::name creator, uint64_t object_id, eosio::name username){
    require_auth(creator);

    whitelist_index whitelist(_me, _me.value);

    objects_index objects(_me, _me.value);
    auto object = objects.find(object_id);

    eosio::check(object != objects.end(), "Object is not found");
    eosio::check(object -> creator_can_emit_new_pieces == true, "Creator cannot emit new pieces and use whitelist");

    auto whitelist_by_object_and_users = whitelist.template get_index<"byobjanduser"_n>();
    auto object_and_user = combine_ids(object_id, username.value);
    auto wl = whitelist_by_object_and_users.find(object_and_user);
  
    eosio::check(wl == whitelist_by_object_and_users.end(), "User is already in a whitelist");

    whitelist.emplace(creator, [&](auto &w){
      w.id = get_global_id("whitelists"_n);
      w.object_id = object_id;
      w.username = username;
    });
  }
  

  [[eosio::action]] void nft::delfromwl(eosio::name creator, uint64_t object_id, eosio::name username){
    require_auth(creator);
    
    whitelist_index whitelist(_me, _me.value);
    
    objects_index objects(_me, _me.value);
    auto object = objects.find(object_id);

    eosio::check(object != objects.end(), "Object is not found");
    eosio::check(object -> creator_can_emit_new_pieces == true, "Creator cannot emit new pieces and use whitelist");

    auto whitelist_by_object_and_users = whitelist.template get_index<"byobjanduser"_n>();
    auto object_and_user = combine_ids(object_id, username.value);

    auto wl = whitelist_by_object_and_users.find(object_and_user);
    
    eosio::check(wl == whitelist_by_object_and_users.end(), "User is not exist in a whitelist");

    whitelist_by_object_and_users.erase(wl);

  }
    


  [[eosio::action]] void nft::setreview(eosio::name buyer, uint64_t object_id, std::string message){
    require_auth(buyer);
  
    pieces_index pieces(_me, _me.value);
    auto object_and_user = combine_ids(object_id, buyer.value);

    auto pieces_by_object_and_users = pieces.template get_index<"byobjanduser"_n>();    
    auto piece = pieces_by_object_and_users.find(object_and_user);
    
    eosio::check(piece != pieces_by_object_and_users.end(), "You dont have pieces for make a review");

    review_index reviews(_me, _me.value);
    auto reviews_by_object_and_users = reviews.template get_index<"byobjanduser"_n>();
    
    auto review = reviews_by_object_and_users.find(object_and_user);
    
    if (review != reviews_by_object_and_users.end()) {
      eosio::check(review -> pieces > piece -> pieces, "You dont have any new pieces for make a new review");
    }

    reviews.emplace(buyer, [&](auto &r) {
      r.id = get_global_id("rewiews"_n);
      r.object_id = object_id;
      r.buyer = buyer;
      r.pieces = piece -> pieces;
      r.review = message;
    });

  }



  [[eosio::action]] void nft::sendmessage(eosio::name username, uint64_t request_id, eosio::name lang, std::string message){
    
    require_auth(username);

    requests_index requests(_me, _me.value);
    auto request = requests.find(request_id);
    eosio::check(request != requests.end(), "Request not found");
    eosio::check(request -> buyer == username || request -> seller == username, "Only buyer or seller can send messages");

    messages_index messages(_me, _me.value);

    messages.emplace(username, [&](auto &m){
      m.id = get_global_id("messages"_n);
      m.request_id = request_id;
      m.lang = lang;
      m.username = username;
      m.message = message;
    });
    

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
          } else if (action == "cancelsell"_n.value) {
            execute_action(name(receiver), name(code), &nft::cancelsell);
          } else if (action == "emit"_n.value) {
            execute_action(name(receiver), name(code), &nft::emit);
          } else if (action == "acceptreq"_n.value) {
            execute_action(name(receiver), name(code), &nft::acceptreq);
          } else if (action == "declinereq"_n.value) {
            execute_action(name(receiver), name(code), &nft::declinereq);
          } else if (action == "cancelreq"_n.value) {
            execute_action(name(receiver), name(code), &nft::cancelreq);
          } else if (action == "payrequest"_n.value) {
            execute_action(name(receiver), name(code), &nft::payrequest);
          } else if (action == "addtowl"_n.value) {
            execute_action(name(receiver), name(code), &nft::addtowl);
          } else if (action == "delfromwl"_n.value) {
            execute_action(name(receiver), name(code), &nft::delfromwl);
          } else if (action == "setreview"_n.value) {
            execute_action(name(receiver), name(code), &nft::setreview);
          } else if (action == "setdelstatus"_n.value) {
            execute_action(name(receiver), name(code), &nft::setdelstatus);
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
