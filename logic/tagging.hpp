#pragma once

#include "empty_types.hpp"
#include "algo.hpp"
#include "logic.hpp"

namespace logic {

#ifdef NOT_NAMESPACED
} // namespace logic
#endif

template<GenericVar X, typename T = typename X::type>
    requires(std::same_as<T, typename X::type>)
struct TaggedValImpl {
    using type = typename X::type;
};

template<GenericVar X>
struct TaggedValImpl<X, int> {
    using type = Int<X>;
};

template<GenericVar X>
using TaggedVal = typename TaggedValImpl<X>::type;

template<FreeVar... FVs>
class Context {
    public:
        Context(Context&) = delete;
        Context(Context&& other) {
            assert(other.active);
                
            other.active = false;
        }
    private:
        Context();
        mutable bool active = true;
        template<typename T, fv_tag_t x, typename... _FVs>
        friend class Tagger;
        template<typename, typename, typename>
        friend class FnDispatcher;
};


template<typename Cont, typename Arguments, Expression... Es>
class PackagedContext { PackagedContext() = delete;};

template<FreeVar... PackagedFVs, FreeVar... AllFVs, Expression... Es>
    requires (
        IsSubset<TypeList<PackagedFVs...>, TypeList<AllFVs...>> &&
        // IsSubset<TypeList<AllFVs...>, Concat<FVsIn<Es>...>> &&
        IsSubset<Concat<FVsIn<Es>...>, TypeList<AllFVs...>>
    )
struct  PackagedContext<Context<AllFVs...>, TypeList<PackagedFVs...>, Es...> {
    
    template<typename... Args>
    constexpr PackagedContext(Context<AllFVs...>&& context, Es... assumptions, Args&&... args)
    : context(std::move(context)), values(std::forward<Args...>(args...)), assumptions(assumptions...) {}

    Context<AllFVs...> context; 
    const std::tuple<TaggedVal<PackagedFVs>...> values;
    const std::tuple<Es...> assumptions;
    template<typename, typename, typename>
    friend class FnDispatcher;
};

template<typename InCont, typename InFVList, typename ExFVList, typename ProdFVList>
struct ResultContextImpl {};

template<FreeVar... InContFVs, FreeVar... InFVs, FreeVar... ExFVs, FreeVar... ProdFVs>
    requires(sizeof...(ExFVs) == sizeof...(InFVs) && IsSubset<TypeList<InFVs...>, TypeList<InContFVs...>>)
struct ResultContextImpl<
    Context<InContFVs...>, TypeList<InFVs...>, TypeList<ExFVs...>, TypeList<ProdFVs...>
> {
    using type = Context<InContFVs...>;
};


//template<typename Inputs, typename Assumptions, typename Output>
//struct FnDispatcher {};


template<FreeVar... ExFVs, Expression... AssEs, FreeVar... ProdFVs, Expression... ProdEs, typename... AllFVs>
    requires(IsSubset<Concat<FVsIn<AssEs>...>, TypeList<ExFVs...>>)
class FnDispatcher<
    Context<ExFVs...>, TypeList<AssEs...>,
    PackagedContext<Context<AllFVs...>, TypeList<ProdFVs...>, ProdEs...>
> {
    public:
        using OutPackage = PackagedContext<Context<AllFVs...>, TypeList<ProdFVs...>, ProdEs...>;

        template<Function<OutPackage, Context<ExFVs...>, TaggedVal<ExFVs>..., AssEs...> Fn>
        FnDispatcher(Fn fn) : fn(fn) {}

        template<typename Cont, typename... InTaggedVals> requires (sizeof...(InTaggedVals) == sizeof...(ExFVs))
        auto operator()(
            Cont,
            EqSubstList<
                AssEs, Zip<TypeList<ExFVs...>,
                TypeList<typename InTaggedVals::TagVar...>, Equals>
            >...,
            InTaggedVals... inputs
        ) {
            // can construct fn's inputs from nothing
            Context<ExFVs...> c_in;
            OutPackage result = fn(std::move(c_in), TaggedVal<ExFVs>(inputs)..., AssEs()...);

            // for each free variable in ProdFVs, that is not in ExFVs, need to add one to output of final context
            
        }
    private:
        const std::function<OutPackage(Context<ExFVs...>, TaggedVal<ExFVs>..., AssEs...)> fn;

};



//template<FreeVar... ExFVs, Expression... AssEs, FreeVar... ProdFVs, Expression... ProdEs, typename... AllFVs>
//    requires(IsSubset<Concat<FVsIn<AssEs>...>, TypeList<ExFVs...>>)
//struct FnDispatcher<
//    Context<ExFVs...>, TypeList<AssEs...>,
//    PackagedContext<Context<AllFVs...>, TypeList<ProdFVs...>, ProdEs...>
//> {
//    using OutPackage = PackagedContext<Context<AllFVs...>, TypeList<ProdFVs...>, ProdEs...>;

//    template<
//        typename Cont, FreeVar... InFVs,
//        Function<OutPackage, Context<ExFVs...>, TaggedVal<ExFVs>..., AssEs...> Fn
//    > requires (sizeof...(InFVs) == sizeof...(ExFVs))
//    static constexpr auto dispatch(
//        Fn fn, Cont&& c, TaggedVal<InFVs>&&... inputs, 
//        EqSubst<AssEs, Zip<TypeList<ExFVs...>, TypeList<InFVs...>, Equals>>... ass
//    ) {
//        auto resolved_fn = 
//            static_cast<std::function<
//                OutPackage(Context<ExFVs...>, TaggedVal<ExFVs>..., AssEs...)
//            >>(fn);
//        return resolved_fn(std::forward(c), std::forward(inputs...), ass...);
//    }
//};


template<typename T, fv_tag_t x, FreeVar... FVs> requires (!IsIn<FV<T, x>, TypeList<FVs...>>)
struct Tagger<T, x, FVs...> {
    using Self = Tagger<T, x, FVs...>;
    static constexpr std::pair<Context<FVs..., FV<T, x>>, TaggedVal<FV<T, x>>> create(Context<FVs...>&& c, T t) {
        TaggedVal<FV<T, x>> tt(t);
        Context<FVs..., FV<T, x>> new_context;
        return std::make_pair(std::move(new_context), std::move(tt));
    }

    template<Variable<T> V>
    static constexpr auto split(Context<FVs...>&& c0, TaggedVal<V> t) {
        auto [c1, tt] = Self::create(std::move(c0), t);
        
        Equals<FV<T, x>, V> eq;
        return std::make_tuple(std::move(c1), tt, eq);
    } 
};

template<typename T, fv_tag_t x, FreeVar... FVs>
    requires (!IsIn<FV<T, x>, TypeList<FVs...>>)
static std::pair<Context<FVs..., FV<T, x>>, TaggedVal<FV<T, x>>> create_tagged(Context<FVs...>&& c, T t) {
    return std::move(Tagger<T, x, FVs...>::create(std::move(c), t));
}

template<typename T, FreeVar... FVs>
static std::pair<
    Context<FVs..., FV<T, UnusedFV<T, TypeList<FVs...>>::tag>>,
    TaggedVal<FV<T, UnusedFV<T, TypeList<FVs...>>::tag>>
> create_tagged(Context<FVs...>&& c, T t) {
    return std::move(Tagger<T, UnusedFV<T, TypeList<FVs...>>::tag, FVs...>::create(std::move(c), t));
}

template<typename Tagged, FreeVar... FVs>
static constexpr auto create_tagged_split(Context<FVs...>&& c0, Tagged t) { 
    using WrappedT = Tagged::Wrapped;
    using Tag = Tagged::TagVar;
    using MyTagger = Tagger<WrappedT, UnusedFV<WrappedT, TypeList<FVs...>>::tag, FVs...>;
    return std::move(MyTagger::template split<Tag>(std::move(c0), t));
};

#ifndef NOT_NAMESPACED
}
#endif