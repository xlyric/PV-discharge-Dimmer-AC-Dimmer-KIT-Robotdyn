<%inherit file="config.tpl"/>
<%namespace name="config" file="config.tpl"/>
<%block name="menu">
${config.generate_menu(support_mqtt=False)}
</%block>
<%block name="content">
${config.generate_content(support_mqtt=False)}
</%block>