#ifndef GIGA_MANAGE

#define GIGA_MANAGE(serializer, property) \
    serializer.manage(property, U(#property))

//    std::cout << "manage " << #property << "\n";

#define GIGA_MANAGE_OPT(serializer, property, defaultValue) serializer.manageOpt(property, U(#property), defaultValue)

#endif
