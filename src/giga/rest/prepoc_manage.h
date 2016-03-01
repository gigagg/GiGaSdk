#ifndef GIGA_MANAGE

#define GIGA_MANAGE(serializer, property) \
    serializer.manage(property, #property)

//    std::cout << U("manage ") << #property << U("\n");

#define GIGA_MANAGE_OPT(serializer, property, defaultValue) serializer.manageOpt(property, #property, defaultValue)

#endif
