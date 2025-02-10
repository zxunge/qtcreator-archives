// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "clangformatutils.h"

#include "clangformatconstants.h"

#include <coreplugin/icore.h>

#include <cppeditor/cppcodestylepreferences.h>
#include <cppeditor/cppcodestylesettings.h>

#include <texteditor/icodestylepreferences.h>
#include <texteditor/tabsettings.h>
#include <texteditor/texteditorsettings.h>

#include <projectexplorer/editorconfiguration.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectmanager.h>

#include <utils/qtcassert.h>
#include <utils/expected.h>
#include <utils/fileutils.h>

#include <QCryptographicHash>
#include <QLoggingCategory>

using namespace clang;
using namespace format;
using namespace llvm;
using namespace CppEditor;
using namespace ProjectExplorer;
using namespace TextEditor;
using namespace Utils;

namespace ClangFormat {

clang::format::FormatStyle calculateQtcStyle()
{
    clang::format::FormatStyle style = getLLVMStyle();
    style.Language = FormatStyle::LK_Cpp;
    style.AccessModifierOffset = -4;
    style.AlignAfterOpenBracket = FormatStyle::BAS_Align;
#if LLVM_VERSION_MAJOR >= 18
    style.AlignConsecutiveAssignments = {false, false, false, false, false, false};
    style.AlignConsecutiveDeclarations = {false, false, false, false, false, false};
#elif LLVM_VERSION_MAJOR >= 15
    style.AlignConsecutiveAssignments = {false, false, false, false, false};
    style.AlignConsecutiveDeclarations = {false, false, false, false, false};
#else
    style.AlignConsecutiveAssignments = FormatStyle::ACS_None;
    style.AlignConsecutiveDeclarations = FormatStyle::ACS_None;
#endif
    style.AlignEscapedNewlines = FormatStyle::ENAS_DontAlign;
    style.AlignOperands = FormatStyle::OAS_Align;
#if LLVM_VERSION_MAJOR >= 16
    style.AlignTrailingComments = {FormatStyle::TCAS_Always, 0};
#else
    style.AlignTrailingComments = true;
#endif
    style.AllowAllParametersOfDeclarationOnNextLine = true;
    style.AllowShortBlocksOnASingleLine = FormatStyle::SBS_Never;
    style.AllowShortCaseLabelsOnASingleLine = false;
    style.AllowShortFunctionsOnASingleLine = FormatStyle::SFS_Inline;
    style.AllowShortIfStatementsOnASingleLine = FormatStyle::SIS_Never;
    style.AllowShortLoopsOnASingleLine = false;
    style.AlwaysBreakBeforeMultilineStrings = false;
#if LLVM_VERSION_MAJOR >= 19
    style.BreakAfterReturnType = FormatStyle::RTBS_None;
    style.BreakTemplateDeclarations = FormatStyle::BTDS_Yes;
#else
    style.AlwaysBreakAfterReturnType = FormatStyle::RTBS_None;
    style.AlwaysBreakTemplateDeclarations = FormatStyle::BTDS_Yes;
#endif
    style.BinPackArguments = false;
#if LLVM_VERSION_MAJOR >= 20
    style.BinPackParameters = FormatStyle::BPPS_OnePerLine;
#else
    style.BinPackParameters = false;
#endif
    style.BraceWrapping.AfterClass = true;
    style.BraceWrapping.AfterControlStatement = FormatStyle::BWACS_Never;
    style.BraceWrapping.AfterEnum = false;
    style.BraceWrapping.AfterFunction = true;
    style.BraceWrapping.AfterNamespace = false;
    style.BraceWrapping.AfterObjCDeclaration = false;
    style.BraceWrapping.AfterStruct = true;
    style.BraceWrapping.AfterUnion = false;
    style.BraceWrapping.BeforeCatch = false;
    style.BraceWrapping.BeforeElse = false;
    style.BraceWrapping.IndentBraces = false;
    style.BraceWrapping.SplitEmptyFunction = false;
    style.BraceWrapping.SplitEmptyRecord = false;
    style.BraceWrapping.SplitEmptyNamespace = false;
    style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
    style.BreakBeforeBraces = FormatStyle::BS_Custom;
    style.BreakBeforeTernaryOperators = true;
    style.BreakConstructorInitializers = FormatStyle::BCIS_BeforeComma;
    style.BreakAfterJavaFieldAnnotations = false;
    style.BreakStringLiterals = true;
    style.ColumnLimit = 100;
    style.CommentPragmas = "^ IWYU pragma:";
    style.CompactNamespaces = false;
#if LLVM_VERSION_MAJOR >= 15
    style.PackConstructorInitializers = FormatStyle::PCIS_BinPack;
#else
    style.ConstructorInitializerAllOnOneLineOrOnePerLine = false;
#endif
    style.ConstructorInitializerIndentWidth = 4;
    style.ContinuationIndentWidth = 4;
    style.Cpp11BracedListStyle = true;
    style.DerivePointerAlignment = false;
    style.DisableFormat = false;
    style.ExperimentalAutoDetectBinPacking = false;
    style.FixNamespaceComments = true;
    style.ForEachMacros = {"forever", "foreach", "Q_FOREACH", "BOOST_FOREACH"};
    style.IncludeStyle.IncludeCategories = {{"^<Q.*", 200, 200, true}};
    style.IncludeStyle.IncludeIsMainRegex = "(Test)?$";
    style.IndentCaseLabels = false;
    style.IndentWidth = 4;
    style.IndentWrappedFunctionNames = false;
    style.JavaScriptQuotes = FormatStyle::JSQS_Leave;
    style.JavaScriptWrapImports = true;
#if LLVM_VERSION_MAJOR >= 19
    style.KeepEmptyLines = {false, false, false};
#else
    style.KeepEmptyLinesAtTheStartOfBlocks = false;
#endif
    // Do not add QT_BEGIN_NAMESPACE/QT_END_NAMESPACE as this will indent lines in between.
    style.MacroBlockBegin = "";
    style.MacroBlockEnd = "";
    style.MaxEmptyLinesToKeep = 1;
    style.NamespaceIndentation = FormatStyle::NI_None;
    style.ObjCBlockIndentWidth = 4;
    style.ObjCSpaceAfterProperty = false;
    style.ObjCSpaceBeforeProtocolList = true;
    style.PenaltyBreakAssignment = 150;
    style.PenaltyBreakBeforeFirstCallParameter = 300;
    style.PenaltyBreakComment = 500;
    style.PenaltyBreakFirstLessLess = 400;
    style.PenaltyBreakString = 600;
    style.PenaltyExcessCharacter = 50;
    style.PenaltyReturnTypeOnItsOwnLine = 300;
    style.PointerAlignment = FormatStyle::PAS_Right;
#if LLVM_VERSION_MAJOR >= 20
    style.ReflowComments = FormatStyle::RCS_Never;
#else
    style.ReflowComments = false;
#endif
    style.SortIncludes = FormatStyle::SI_CaseSensitive;
#if LLVM_VERSION_MAJOR >= 16
    style.SortUsingDeclarations = FormatStyle::SUD_Lexicographic;
#else
    style.SortUsingDeclarations = true;
#endif
    style.SpaceAfterCStyleCast = true;
    style.SpaceAfterTemplateKeyword = false;
    style.SpaceBeforeAssignmentOperators = true;
    style.SpaceBeforeParens = FormatStyle::SBPO_ControlStatements;
#if LLVM_VERSION_MAJOR < 17
    style.SpaceInEmptyParentheses = false;
#endif
    style.SpacesBeforeTrailingComments = 1;
    style.SpacesInAngles = FormatStyle::SIAS_Never;
    style.SpacesInContainerLiterals = false;
#if LLVM_VERSION_MAJOR >= 17
    style.SpacesInParens = FormatStyle::SIPO_Never;
#else
    style.SpacesInCStyleCastParentheses = false;
    style.SpacesInParentheses = false;
#endif
    style.SpacesInSquareBrackets = false;
    addQtcStatementMacros(style);
    style.TabWidth = 4;
    style.UseTab = FormatStyle::UT_Never;
    style.Standard = FormatStyle::LS_Auto;
    return style;
}

clang::format::FormatStyle qtcStyle()
{
    static clang::format::FormatStyle style = calculateQtcStyle();
    return style;
}

clang::format::FormatStyle currentQtStyle(const TextEditor::ICodeStylePreferences *preferences)
{
    clang::format::FormatStyle style = qtcStyle();
    if (!preferences)
        return style;

    fromTabSettings(style, preferences->tabSettings());
    if (auto ccpPreferences = dynamic_cast<const CppEditor::CppCodeStylePreferences *>(preferences))
        fromCppCodeStyleSettings(style, ccpPreferences->codeStyleSettings());
    return style;
}

void fromCppCodeStyleSettings(clang::format::FormatStyle &style,
                              const CppEditor::CppCodeStyleSettings &settings)
{
    using namespace clang::format;
    if (settings.indentAccessSpecifiers)
        style.AccessModifierOffset = 0;
    else
        style.AccessModifierOffset = -1 * style.IndentWidth;

    if (settings.indentNamespaceBody && settings.indentNamespaceBraces)
        style.NamespaceIndentation = FormatStyle::NamespaceIndentationKind::NI_All;
    else
        style.NamespaceIndentation = FormatStyle::NamespaceIndentationKind::NI_None;

    if (settings.indentClassBraces && settings.indentEnumBraces && settings.indentBlockBraces
        && settings.indentFunctionBraces)
        style.BreakBeforeBraces = FormatStyle::BS_Whitesmiths;
    else
        style.BreakBeforeBraces = FormatStyle::BS_Custom;

    style.IndentCaseLabels = settings.indentSwitchLabels;
    style.IndentCaseBlocks = settings.indentBlocksRelativeToSwitchLabels;

    if (settings.extraPaddingForConditionsIfConfusingAlign)
        style.BreakBeforeBinaryOperators = FormatStyle::BOS_All;
    else if (settings.alignAssignments)
        style.BreakBeforeBinaryOperators = FormatStyle::BOS_NonAssignment;
    else
        style.BreakBeforeBinaryOperators = FormatStyle::BOS_None;

    style.DerivePointerAlignment = settings.bindStarToIdentifier || settings.bindStarToTypeName
                                   || settings.bindStarToLeftSpecifier
                                   || settings.bindStarToRightSpecifier;

    if ((settings.bindStarToIdentifier || settings.bindStarToRightSpecifier)
        && ClangFormatSettings::instance().mode() == ClangFormatSettings::Mode::Formatting)
        style.PointerAlignment = FormatStyle::PAS_Right;

    if ((settings.bindStarToTypeName || settings.bindStarToLeftSpecifier)
        && ClangFormatSettings::instance().mode() == ClangFormatSettings::Mode::Formatting)
        style.PointerAlignment = FormatStyle::PAS_Left;
}

void fromTabSettings(clang::format::FormatStyle &style, const TextEditor::TabSettings &settings)
{
    using namespace clang::format;

    style.IndentWidth = settings.m_indentSize;
    style.TabWidth = settings.m_tabSize;

    switch (settings.m_tabPolicy) {
    case TextEditor::TabSettings::TabPolicy::MixedTabPolicy:
        style.UseTab = FormatStyle::UT_ForContinuationAndIndentation;
        break;
    case TextEditor::TabSettings::TabPolicy::SpacesOnlyTabPolicy:
        style.UseTab = FormatStyle::UT_Never;
        break;
    case TextEditor::TabSettings::TabPolicy::TabsOnlyTabPolicy:
        style.UseTab = FormatStyle::UT_Always;
        break;
    }
}

QString projectUniqueId(ProjectExplorer::Project *project)
{
    if (!project)
        return QString();

    return QString::fromUtf8(QCryptographicHash::hash(project->projectFilePath().toString().toUtf8(),
                                                      QCryptographicHash::Md5)
                                 .toHex(0));
}

bool getProjectUseGlobalSettings(const ProjectExplorer::Project *project)
{
    const QVariant projectUseGlobalSettings = project ? project->namedSettings(
                                                  Constants::USE_GLOBAL_SETTINGS)
                                                      : QVariant();

    return projectUseGlobalSettings.isValid() ? projectUseGlobalSettings.toBool() : true;
}

bool getProjectCustomSettings(const ProjectExplorer::Project *project)
{
    const QVariant projectCustomSettings = project ? project->namedSettings(
                                               Constants::USE_CUSTOM_SETTINGS_ID)
                                                   : QVariant();

    return projectCustomSettings.isValid()
               ? projectCustomSettings.toBool()
               : ClangFormatSettings::instance().useCustomSettings();
}

bool getCurrentCustomSettings(const Utils::FilePath &filePath)
{
    const ProjectExplorer::Project *project = ProjectExplorer::ProjectManager::projectForFile(
        filePath);

    return getProjectUseGlobalSettings(project)
               ? ClangFormatSettings::instance().useCustomSettings()
               : getProjectCustomSettings(project);
}

ClangFormatSettings::Mode getProjectIndentationOrFormattingSettings(
    const ProjectExplorer::Project *project)
{
    const QVariant projectIndentationOrFormatting = project
                                                        ? project->namedSettings(Constants::MODE_ID)
                                                        : QVariant();

    return projectIndentationOrFormatting.isValid()
               ? static_cast<ClangFormatSettings::Mode>(projectIndentationOrFormatting.toInt())
               : ClangFormatSettings::instance().mode();
}

ClangFormatSettings::Mode getCurrentIndentationOrFormattingSettings(const Utils::FilePath &filePath)
{
    const ProjectExplorer::Project *project = ProjectExplorer::ProjectManager::projectForFile(
        filePath);

    return getProjectUseGlobalSettings(project)
               ? ClangFormatSettings::instance().mode()
               : getProjectIndentationOrFormattingSettings(project);
}

Utils::FilePath findConfig(const Utils::FilePath &filePath)
{
    Utils::FilePath parentDirectory = filePath.parentDir();
    while (parentDirectory.exists()) {
        Utils::FilePath settingsFilePath = parentDirectory / Constants::SETTINGS_FILE_NAME;
        if (settingsFilePath.exists())
            return settingsFilePath;

        Utils::FilePath settingsAltFilePath = parentDirectory / Constants::SETTINGS_FILE_ALT_NAME;
        if (settingsAltFilePath.exists())
            return settingsAltFilePath;

        parentDirectory = parentDirectory.parentDir();
    }
    return {};
}

ICodeStylePreferences *preferencesForFile(const Utils::FilePath &filePath)
{
    const ProjectExplorer::Project *project = ProjectExplorer::ProjectManager::projectForFile(
        filePath);

    return !getProjectUseGlobalSettings(project) && project
               ? project->editorConfiguration()->codeStyle("Cpp")->currentPreferences()
               : TextEditor::TextEditorSettings::codeStyle("Cpp")->currentPreferences();
}

Utils::FilePath configForFile(const Utils::FilePath &filePath)
{
    if (!getCurrentCustomSettings(filePath))
        return findConfig(filePath);

    const TextEditor::ICodeStylePreferences *preferences = preferencesForFile(filePath);
    return filePathToCurrentSettings(preferences);
}

void addQtcStatementMacros(clang::format::FormatStyle &style)
{
    static const std::vector<std::string> macros = {"Q_CLASSINFO",
                                                    "Q_ENUM",
                                                    "Q_ENUM_NS",
                                                    "Q_FLAG",
                                                    "Q_FLAG_NS",
                                                    "Q_GADGET",
                                                    "Q_GADGET_EXPORT",
                                                    "Q_INTERFACES",
                                                    "Q_LOGGING_CATEGORY",
                                                    "Q_MOC_INCLUDE",
                                                    "Q_NAMESPACE",
                                                    "Q_NAMESPACE_EXPORT",
                                                    "Q_OBJECT",
                                                    "Q_PROPERTY",
                                                    "Q_REVISION",
                                                    "Q_DISABLE_COPY",
                                                    "Q_SET_OBJECT_NAME",
                                                    "QT_BEGIN_NAMESPACE",
                                                    "QT_END_NAMESPACE",

                                                    "QML_ADDED_IN_MINOR_VERSION",
                                                    "QML_ANONYMOUS",
                                                    "QML_ATTACHED",
                                                    "QML_DECLARE_TYPE",
                                                    "QML_DECLARE_TYPEINFO",
                                                    "QML_ELEMENT",
                                                    "QML_EXTENDED",
                                                    "QML_EXTENDED_NAMESPACE",
                                                    "QML_EXTRA_VERSION",
                                                    "QML_FOREIGN",
                                                    "QML_FOREIGN_NAMESPACE",
                                                    "QML_IMPLEMENTS_INTERFACES",
                                                    "QML_INTERFACE",
                                                    "QML_NAMED_ELEMENT",
                                                    "QML_REMOVED_IN_MINOR_VERSION",
                                                    "QML_SINGLETON",
                                                    "QML_UNAVAILABLE",
                                                    "QML_UNCREATABLE",
                                                    "QML_VALUE_TYPE"};
    for (const std::string &macro : macros) {
        if (std::find(style.StatementMacros.begin(), style.StatementMacros.end(), macro)
            == style.StatementMacros.end())
            style.StatementMacros.emplace_back(macro);
    }

    const std::vector<std::string> emitMacros = {"emit", "Q_EMIT"};
    for (const std::string &emitMacro : emitMacros) {
        if (std::find(
                style.StatementAttributeLikeMacros.begin(),
                style.StatementAttributeLikeMacros.end(),
                emitMacro)
            == style.StatementAttributeLikeMacros.end())
            style.StatementAttributeLikeMacros.push_back(emitMacro);
    }
}

Utils::FilePath filePathToCurrentSettings(const TextEditor::ICodeStylePreferences *codeStyle)
{
    return Core::ICore::userResourcePath() / "clang-format/"
           / Utils::FileUtils::fileSystemFriendlyName(codeStyle->displayName())
           / QLatin1String(Constants::SETTINGS_FILE_NAME);
}

Utils::expected_str<void> parseConfigurationContent(const std::string &fileContent,
                                                    clang::format::FormatStyle &style,
                                                    bool allowUnknownOptions)
{
    llvm::SourceMgr::DiagHandlerTy diagHandler = [](const llvm::SMDiagnostic &diag, void *context) {
        QString *errorMessage = reinterpret_cast<QString *>(context);
        *errorMessage = QString::fromStdString(diag.getMessage().str()) + " "
                        + QString::number(diag.getLineNo()) + ":"
                        + QString::number(diag.getColumnNo());
    };

    QString errorMessage;
    style.Language = clang::format::FormatStyle::LK_Cpp;
    const std::error_code error = parseConfiguration(
        llvm::MemoryBufferRef(fileContent, "YAML"),
        &style,
        allowUnknownOptions,
        diagHandler,
        &errorMessage);

    errorMessage = errorMessage.trimmed().isEmpty() ? QString::fromStdString(error.message())
                                                    : errorMessage;
    if (error)
        return make_unexpected(errorMessage);
    return {};
}

Utils::expected_str<void> parseConfigurationFile(const Utils::FilePath &filePath,
                                                 clang::format::FormatStyle &style)
{
    return parseConfigurationContent(filePath.fileContents().value_or(QByteArray()).toStdString(),
                                     style, true);
}

} // namespace ClangFormat
