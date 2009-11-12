/* BotServ core functions
 *
 * (C) 2003-2009 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * $Id$
 *
 */
/*************************************************************************/

#include "module.h"

class CommandBSSay : public Command
{
 public:
	CommandBSSay() : Command("SAY", 2, 2)
	{
	}

	CommandReturn Execute(User *u, const std::vector<ci::string> &params)
	{
		ChannelInfo *ci;

		const char *chan = params[0].c_str();
		const char *text = params[1].c_str();

		ci = cs_findchan(chan);

		if (!check_access(u, ci, CA_SAY))
		{
			notice_lang(s_BotServ, u, ACCESS_DENIED);
			return MOD_CONT;
		}

		if (!ci->bi)
		{
			notice_help(s_BotServ, u, BOT_NOT_ASSIGNED);
			return MOD_CONT;
		}

		if (!ci->c || ci->c->usercount < BSMinUsers)
		{
			notice_lang(s_BotServ, u, BOT_NOT_ON_CHANNEL, ci->name);
			return MOD_CONT;
		}

		if (text[0] == '\001')
		{
			this->OnSyntaxError(u, "");
			return MOD_CONT;
		}

		ircdproto->SendPrivmsg(ci->bi, ci->name, "%s", text);
		ci->bi->lastmsg = time(NULL);
		if (LogBot && LogChannel && LogChan && !debug && findchan(LogChannel))
			ircdproto->SendPrivmsg(ci->bi, LogChannel, "SAY %s %s %s", u->nick, ci->name, text);
		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &subcommand)
	{
		notice_help(s_BotServ, u, BOT_HELP_SAY);
		return true;
	}

	void OnSyntaxError(User *u, const ci::string &subcommand)
	{
		syntax_error(s_BotServ, u, "SAY", BOT_SAY_SYNTAX);
	}
};

class BSSay : public Module
{
 public:
	BSSay(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(CORE);
		this->AddCommand(BOTSERV, new CommandBSSay());

		ModuleManager::Attach(I_OnBotServHelp, this);
	}
	void OnBotServHelp(User *u)
	{
		notice_lang(s_BotServ, u, BOT_HELP_CMD_SAY);
	}
};

MODULE_INIT(BSSay)
