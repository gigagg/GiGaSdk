#ifndef GIGA_MANAGE

#define GIGA_MANAGE(serializer, property) \
    std::cout << "manage " << #property << "\n"; \
    serializer.manage(property, #property)

#define GIGA_MANAGE_OPT(serializer, property, defaultValue) serializer.manageOpt(property, #property, defaultValue)

#endif
