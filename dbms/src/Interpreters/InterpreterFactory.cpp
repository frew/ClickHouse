#include <Parsers/ASTAlterQuery.h>
#include <Parsers/ASTCheckQuery.h>
#include <Parsers/ASTCreateQuery.h>
#include <Parsers/ASTCreateUserQuery.h>
#include <Parsers/ASTCreateRoleQuery.h>
#include <Parsers/ASTCreateQuotaQuery.h>
#include <Parsers/ASTCreateRowPolicyQuery.h>
#include <Parsers/ASTDropAccessEntityQuery.h>
#include <Parsers/ASTDropQuery.h>
#include <Parsers/ASTInsertQuery.h>
#include <Parsers/ASTKillQueryQuery.h>
#include <Parsers/ASTOptimizeQuery.h>
#include <Parsers/ASTRenameQuery.h>
#include <Parsers/ASTSelectQuery.h>
#include <Parsers/ASTSelectWithUnionQuery.h>
#include <Parsers/ASTSetQuery.h>
#include <Parsers/ASTSetRoleQuery.h>
#include <Parsers/ASTShowCreateAccessEntityQuery.h>
#include <Parsers/ASTShowProcesslistQuery.h>
#include <Parsers/ASTShowGrantsQuery.h>
#include <Parsers/ASTShowQuotasQuery.h>
#include <Parsers/ASTShowRowPoliciesQuery.h>
#include <Parsers/ASTShowTablesQuery.h>
#include <Parsers/ASTUseQuery.h>
#include <Parsers/ASTExplainQuery.h>
#include <Parsers/TablePropertiesQueriesASTs.h>
#include <Parsers/ASTWatchQuery.h>
#include <Parsers/ASTGrantQuery.h>

#include <Interpreters/InterpreterAlterQuery.h>
#include <Interpreters/InterpreterCheckQuery.h>
#include <Interpreters/InterpreterCreateQuery.h>
#include <Interpreters/InterpreterCreateUserQuery.h>
#include <Interpreters/InterpreterCreateRoleQuery.h>
#include <Interpreters/InterpreterCreateQuotaQuery.h>
#include <Interpreters/InterpreterCreateRowPolicyQuery.h>
#include <Interpreters/InterpreterDescribeQuery.h>
#include <Interpreters/InterpreterExplainQuery.h>
#include <Interpreters/InterpreterDropAccessEntityQuery.h>
#include <Interpreters/InterpreterDropQuery.h>
#include <Interpreters/InterpreterExistsQuery.h>
#include <Interpreters/InterpreterFactory.h>
#include <Interpreters/InterpreterInsertQuery.h>
#include <Interpreters/InterpreterKillQueryQuery.h>
#include <Interpreters/InterpreterOptimizeQuery.h>
#include <Interpreters/InterpreterRenameQuery.h>
#include <Interpreters/InterpreterSelectQuery.h>
#include <Interpreters/InterpreterSelectWithUnionQuery.h>
#include <Interpreters/InterpreterSetQuery.h>
#include <Interpreters/InterpreterSetRoleQuery.h>
#include <Interpreters/InterpreterShowCreateAccessEntityQuery.h>
#include <Interpreters/InterpreterShowCreateQuery.h>
#include <Interpreters/InterpreterShowProcesslistQuery.h>
#include <Interpreters/InterpreterShowGrantsQuery.h>
#include <Interpreters/InterpreterShowQuotasQuery.h>
#include <Interpreters/InterpreterShowRowPoliciesQuery.h>
#include <Interpreters/InterpreterShowTablesQuery.h>
#include <Interpreters/InterpreterSystemQuery.h>
#include <Interpreters/InterpreterUseQuery.h>
#include <Interpreters/InterpreterWatchQuery.h>
#include <Interpreters/InterpreterGrantQuery.h>

#include <Parsers/ASTSystemQuery.h>

#include <Common/typeid_cast.h>
#include <Common/ProfileEvents.h>


namespace ProfileEvents
{
    extern const Event Query;
    extern const Event SelectQuery;
    extern const Event InsertQuery;
}


namespace DB
{
namespace ErrorCodes
{
    extern const int READONLY;
    extern const int UNKNOWN_TYPE_OF_QUERY;
    extern const int QUERY_IS_PROHIBITED;
}


std::unique_ptr<IInterpreter> InterpreterFactory::get(ASTPtr & query, Context & context, QueryProcessingStage::Enum stage)
{
    ProfileEvents::increment(ProfileEvents::Query);

    if (query->as<ASTSelectQuery>())
    {
        /// This is internal part of ASTSelectWithUnionQuery.
        /// Even if there is SELECT without union, it is represented by ASTSelectWithUnionQuery with single ASTSelectQuery as a child.
        return std::make_unique<InterpreterSelectQuery>(query, context, SelectQueryOptions(stage));
    }
    else if (query->as<ASTSelectWithUnionQuery>())
    {
        ProfileEvents::increment(ProfileEvents::SelectQuery);
        return std::make_unique<InterpreterSelectWithUnionQuery>(query, context, SelectQueryOptions(stage));
    }
    else if (query->as<ASTInsertQuery>())
    {
        ProfileEvents::increment(ProfileEvents::InsertQuery);
        bool allow_materialized = static_cast<bool>(context.getSettingsRef().insert_allow_materialized_columns);
        return std::make_unique<InterpreterInsertQuery>(query, context, allow_materialized);
    }
    else if (query->as<ASTCreateQuery>())
    {
        return std::make_unique<InterpreterCreateQuery>(query, context);
    }
    else if (query->as<ASTDropQuery>())
    {
        return std::make_unique<InterpreterDropQuery>(query, context);
    }
    else if (query->as<ASTRenameQuery>())
    {
        return std::make_unique<InterpreterRenameQuery>(query, context);
    }
    else if (query->as<ASTShowTablesQuery>())
    {
        return std::make_unique<InterpreterShowTablesQuery>(query, context);
    }
    else if (query->as<ASTUseQuery>())
    {
        return std::make_unique<InterpreterUseQuery>(query, context);
    }
    else if (query->as<ASTSetQuery>())
    {
        /// readonly is checked inside InterpreterSetQuery
        return std::make_unique<InterpreterSetQuery>(query, context);
    }
    else if (query->as<ASTSetRoleQuery>())
    {
        return std::make_unique<InterpreterSetRoleQuery>(query, context);
    }
    else if (query->as<ASTOptimizeQuery>())
    {
        return std::make_unique<InterpreterOptimizeQuery>(query, context);
    }
    else if (query->as<ASTExistsTableQuery>())
    {
        return std::make_unique<InterpreterExistsQuery>(query, context);
    }
    else if (query->as<ASTExistsDictionaryQuery>())
    {
        return std::make_unique<InterpreterExistsQuery>(query, context);
    }
    else if (query->as<ASTShowCreateTableQuery>())
    {
        return std::make_unique<InterpreterShowCreateQuery>(query, context);
    }
    else if (query->as<ASTShowCreateDatabaseQuery>())
    {
        return std::make_unique<InterpreterShowCreateQuery>(query, context);
    }
    else if (query->as<ASTShowCreateDictionaryQuery>())
    {
        return std::make_unique<InterpreterShowCreateQuery>(query, context);
    }
    else if (query->as<ASTDescribeQuery>())
    {
        return std::make_unique<InterpreterDescribeQuery>(query, context);
    }
    else if (query->as<ASTExplainQuery>())
    {
        return std::make_unique<InterpreterExplainQuery>(query, context);
    }
    else if (query->as<ASTShowProcesslistQuery>())
    {
        return std::make_unique<InterpreterShowProcesslistQuery>(query, context);
    }
    else if (query->as<ASTAlterQuery>())
    {
        return std::make_unique<InterpreterAlterQuery>(query, context);
    }
    else if (query->as<ASTCheckQuery>())
    {
        return std::make_unique<InterpreterCheckQuery>(query, context);
    }
    else if (query->as<ASTKillQueryQuery>())
    {
        return std::make_unique<InterpreterKillQueryQuery>(query, context);
    }
    else if (query->as<ASTSystemQuery>())
    {
        return std::make_unique<InterpreterSystemQuery>(query, context);
    }
    else if (query->as<ASTWatchQuery>())
    {
        return std::make_unique<InterpreterWatchQuery>(query, context);
    }
    else if (query->as<ASTCreateUserQuery>())
    {
        return std::make_unique<InterpreterCreateUserQuery>(query, context);
    }
    else if (query->as<ASTCreateRoleQuery>())
    {
        return std::make_unique<InterpreterCreateRoleQuery>(query, context);
    }
    else if (query->as<ASTCreateQuotaQuery>())
    {
        return std::make_unique<InterpreterCreateQuotaQuery>(query, context);
    }
    else if (query->as<ASTCreateRowPolicyQuery>())
    {
        return std::make_unique<InterpreterCreateRowPolicyQuery>(query, context);
    }
    else if (query->as<ASTDropAccessEntityQuery>())
    {
        return std::make_unique<InterpreterDropAccessEntityQuery>(query, context);
    }
    else if (query->as<ASTGrantQuery>())
    {
        return std::make_unique<InterpreterGrantQuery>(query, context);
    }
    else if (query->as<ASTShowCreateAccessEntityQuery>())
    {
        return std::make_unique<InterpreterShowCreateAccessEntityQuery>(query, context);
    }
    else if (query->as<ASTShowGrantsQuery>())
    {
        return std::make_unique<InterpreterShowGrantsQuery>(query, context);
    }
    else if (query->as<ASTShowQuotasQuery>())
    {
        return std::make_unique<InterpreterShowQuotasQuery>(query, context);
    }
    else if (query->as<ASTShowRowPoliciesQuery>())
    {
        return std::make_unique<InterpreterShowRowPoliciesQuery>(query, context);
    }
    else
        throw Exception("Unknown type of query: " + query->getID(), ErrorCodes::UNKNOWN_TYPE_OF_QUERY);
}
}
